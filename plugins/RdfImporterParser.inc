<?php

define('RDFIMPORTER_PARSER_MAX_SAMPLES', 4);

/**
 * Class definition for RdfImporterParser
 */
class RdfImporterParser extends FeedsParser {

  public function parse(FeedsImportBatch $batch, FeedsSource $source) {
    
    // Get the current site language to use data
    // from that language where possible
    global $language;
    
    $individuals = $batch->getRaw();
    
    foreach ($individuals as $uri => $individual) {
      $individuals[$uri] = new RdfImporterIndividual($uri, $batch->arc, $language->language);
    }

    $this->gatherSamples($individuals, $batch);
    $this->saveSamples($source, $batch->samples, $batch->uri_list);
    
    $batch->setItems($individuals);
  }
  
  /**
   * Implementation of FeedsParser::getMappingSources().
   *
   * @todo Need to give some guidance if there are no samples/sources available
   */
  public function getMappingSources() {
    $sources = array();
    $samples = $this->loadSamples();
    
    foreach ($samples as $uri => $property) {
      // When sample arrays are merged, the label can become an array instead of a string
      $name = is_array($property['label']) ? $property['label'][0] : $property['label'];
      $sources[$uri] = array('name' => t($name), 'description' => t($uri));
    }

    uasort($sources, array($this, 'sortSources'));
    $sources = array_reverse($sources);
    
    $sources['label'] = array('name' => t('Resource label'), 'description' => t('Single label for a resource'));
    $sources['uri'] = array('name' => t('Resource URI'), 'description' => t('URI for a resource'));
    
    return array_reverse($sources);
  }
  
  /**
   * Implementation of FeedsParser::getSourceElement().
   */
  public function getSourceElement($individual, $source) {
    if ($source == 'uri') {
      return $individual->getUri();
    }
    if ($source == 'label') {
      return $individual->getLabel();
    }
    $properties = $individual->getProperties();
    return isset($properties[$source]) ? $properties[$source] : '';
  }
  
  /**
   * Save sample data to the current $source config so it gets saved to the database
   */
  public function saveSamples($source, $samples, $uri_list) {
    $data = array('uri_list' => $uri_list, 'samples' => $samples, 'last_updated' => time());
    $config = $source->getConfig();
    $config['RdfImporterParser'] = $data;
    $source->setConfig($config);
  }
  
  /**
   * Load samples for all feed nodes that use the current importer
   */
  public function loadSamples() {
    $samples = array();
    $query = db_query("SELECT feed_nid FROM {feeds_source} WHERE id = '%s'", $this->id);
    while($result = db_fetch_object($query)) {
      $feed_source = feeds_source($this->id, $result->feed_nid);
      $config = $feed_source->getConfigFor($this);
      if (isset($config['samples'])) {
        $samples = array_merge_recursive($samples, $config['samples']);
      }
    }
    return $samples;
  }
  
  /**
   * Gather sample data for all properties throughout the import
   * and save it in the $batch
   */
  public function gatherSamples($individuals, FeedsImportBatch $batch) {
    $samples = ($batch->samples) ? $batch->samples : array();
    $max_samples = RDFIMPORTER_PARSER_MAX_SAMPLES;
    
    foreach ($individuals as $individual) {
      
      foreach ($individual->getSamples() as $uri => $item) {
        
        // Only add samples for this property if it hasn't been saved yet or we still need more values
        if (!isset($samples[$uri]) || (count($samples[$uri]['values']) < $max_samples)) {
          
          if (!isset($samples[$uri])) {
            // Try to get a label for each property URI, otherwise make one
            $batch->arc->setURI($uri);
            $label = $batch->arc->getPropValue(RDFS_LABEL_URI);
            $label = ($label) ? $label : $batch->arc->extractTermLabel($uri);
            $samples[$uri] = array();
            $samples[$uri]['label'] = $label;
            $samples[$uri]['values'] = array();
          }
          
          $count = count($samples[$uri]['values']);
          
          // Fetch a label for object properties
          if ($item->getType() == 'uri') {
            $label = $item->getLabel();
            $label = $this->trimSample($label);
            $value = $item->getValue();
          }
          else {
            $label = '';
            $value = $this->trimSample($item->getValue());
          }
          
          // Reduce duplicates by using hashes as keys
          $key = md5(serialize($value));
          
          $samples[$uri]['values'][$key]['type'] = $item->getType();
          $samples[$uri]['values'][$key]['value'] = $value;
          $samples[$uri]['values'][$key]['label'] = $label; 
          
          $count = count($samples[$uri]['values']);
          if ($count == $max_samples) {
            // Strip out the hash keys
            $samples[$uri]['values'] = array_values($samples[$uri]['values']);
            // Finished with this property
            break;
          } 
        }
      }
    }
    $batch->samples = $samples;
  }
  
  /**
   * Helper function to keep sample data short
   */
  public function trimSample($string, $max = 80, $delimeter = '...') { 
    
    // If the string is a URL just return it whole
    if (rdfimporter_validate_uri($string)) return $string;
    
    $string = check_plain($string);
    if (strlen($string) > $max) { 
       preg_match('/(.{' . $max . '}.*?)\b/', $string, $matches); 
       return rtrim($matches[1]) . $delimeter; 
    } 
    else { 
       return $string; 
    } 
  }
  
  /**
   * uasort() callback for sorting the array of mapping sources
   */
  public function sortSources($a, $b) {
    return strcasecmp($a['name'], $b['name']);
  }
  
  /**
   * Add form fields so we're able to save sample data to the feed source
   * in the database. These fields are only values and are not visible in forms
   */
  public function sourceForm($source_config) {
    $form = array();
    $form['uri_list'] = array(
      '#type' => 'value',
      '#value' => isset($source_config['uri_list']) ? $source_config['uri_list'] : '',
    );
    $form['samples'] = array(
      '#type' => 'value',
      '#value' => isset($source_config['samples']) ? $source_config['samples'] : '',
    );
    $form['last_updated'] = array(
      '#type' => 'value',
      '#value' => isset($source_config['last_updated']) ? $source_config['last_updated'] : '',
    );
    return $form;
  }
}
