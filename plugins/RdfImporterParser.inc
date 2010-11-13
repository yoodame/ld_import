<?php
// $Id$

/**
 * @file
 * Home of the RdfImporterParser.
 *
 * This is where the feed preview data is assembled
 * and where the form to display that data gets constructed.
 *
 * Properties available for mapping are determined by the
 * properties in the feed preview
 */
 
define('RDFIMPORTER_MAX_SAMPLES', 4);
define('RDFIMPORTER_PREVIEW_FETCH_LIMIT', 0);

/**
 * Class definition for RdfImporterParser.
 */
class RdfImporterParser extends FeedsParser {

  /**
   * Implementation of FeedsParser::parse().
   */
  public function parse(FeedsImportBatch $batch, FeedsSource $source) {
    $individuals = $batch->getRaw();
    // Get the current site language to limit content
    // to that language where possible
    global $language;
    $batch->language = $language->language;
    $batch->setItems($individuals);
  }
  
  /**
   * Implementation of FeedsParser::getMappingSources().
   *
   * @todo Need to give some guidance if there are no samples/sources available.
   */
  public function getMappingSources() {
    $sources = array();
    $samples = $this->loadSamples();
    
    foreach ($samples as $uri => $property) {
      // When sample arrays get merged, the label can become an array instead of a string.
      $name = is_array($property['label']) ? $property['label'][0] : $property['label'];
      
      if ($item = reset($property['values'])) {
        switch ($item['type']) {
          case 'uri':
          case 'bnode':
            $key = $this->createMappingID($uri);
            $sources[$key] = array('name' => t($name . ' (URIs)'), 'description' => t($uri));
            $key = $this->createMappingID($uri, TRUE);
            $sources[$key] = array('name' => t($name . ' (labels)'), 'description' => t($uri));
            break;
          case 'literal':
          default:
            $key = $this->createMappingID($uri);
            $sources[$key] = array('name' => t($name), 'description' => t($uri));
            break;
        }
      }
    }

    uasort($sources, array($this, 'sortSources'));
    $sources = array_reverse($sources);
    
    $sources['label'] = array('name' => t('Resource label'), 'description' => t('Single label for a resource'));
    $sources['uri'] = array('name' => t('Resource URI'), 'description' => t('URI for a resource'));
    
    drupal_alter('rdfimporter_parser_sources', $sources, feeds_importer($this->id)->config['content_type']);
    
    // dsm($sources);
    
    return array_reverse($sources);
  }
  
  /**
   * Create a serialized source array to represent an RDF property
   */
  function createMappingID($uri, $label = FALSE) {
    if ($label) {
      return serialize(array($uri => array(RDFS_LABEL_URI => array())));
    }
    else {
      return serialize(array($uri => array()));
    }
  }
  
  /**
   * Implementation of FeedsParser::getSourceElement().
   */
  public function getSourceElement($batch, $source) {
    $individual = $batch->currentItem();
    $items = array();
    
    if ($source == 'uri') {
      $items[] = $individual->getUri();
    }
    if ($source == 'label') {
      $items[] = $individual->getLabel();
    }
    $source = @unserialize($source);
    if (is_array($source)) {
      $this->extractPropertyValues($individual, $source, $items);
      sort($items);
    }
    return $items;
  }
  
  public function extractPropertyValues(RdfImporterIndividual $individual, $source, &$items) {

    foreach (element_children($source) as $property_uri) {

      if ($property = $individual->getProperty($property_uri)) {

        foreach ($property->getValues() as $item) {

          // If there are deeper nested properties, continue to drill down.
          if ($item->getType() == 'uri' && element_children($source[$property_uri])) {
            $target_individual = new RdfImporterIndividual($item->getValue(), $individual->arc, $individual->language);
            $this->extractPropertyValues($target_individual, $source[$property_uri], $items);
          }
          else {
            $items[] = $item->getValue();
          }
        }
      }
    }
  }
  
  /**
   * Save sample data to the current $source config so it gets saved to the database.
   */
  public function saveSamples(FeedsImportBatch $batch, FeedsSource $source) {
    $config = $source->getConfig();
    $class = get_class($this);
    $config[$class]['uri_list'] = $batch->uri_list;
    $config[$class]['samples'] = $batch->samples;
    $config[$class]['last_updated'] = time();
    $source->setConfig($config);
    $source->save();
  }
  
  /**
   * Load samples for all feed nodes that use the current importer.
   */
  public function loadSamples() {
    $samples = array();
    $query = db_query("SELECT feed_nid FROM {feeds_source} WHERE id = '%s'", $this->id);
    while ($result = db_fetch_object($query)) {
      $feed_source = feeds_source($this->id, $result->feed_nid);
      $config = $feed_source->getConfigFor($this);
      if (is_array($config['samples'])) {
        $samples = array_merge_recursive($samples, $config['samples']);
      }
    }
    return $samples;
  }
  
  /**
   * Gather sample data for all an individual's properties and save it in the batch.
   */
  public function gatherSamples(FeedsImportBatch $batch) {
    $samples = ($batch->samples) ? $batch->samples : array();
    $max_samples = ($batch->sample_limit) ? $batch->sample_limit : RDFIMPORTER_MAX_SAMPLES;
    
    $individual = $batch->shiftItem();
    
    foreach ($individual->getSamples() as $uri => $item) {
      
      // Only add samples for this property if it hasn't been saved yet or we still need more values
      if (!isset($samples[$uri]) || (count($samples[$uri]['values']) < $max_samples)) {
        
        if (!isset($samples[$uri])) {
          $batch->arc->setURI($uri);
          
          // If enabled, try to get a label for each property URI, otherwise make one.
          if ($batch->fetch_labels) {
            $label = $batch->arc->getPropValue(RDFS_LABEL_URI);
            $label = ($label) ? $label : $batch->arc->extractTermLabel($uri);
          }
          else {
            $label = $batch->arc->extractTermLabel($uri);
          }
          
          $samples[$uri] = array();
          $samples[$uri]['label'] = $label;
          $samples[$uri]['values'] = array();
        }
        
        $label = '';
        $value = '';
        
        // Fetch a label for object properties only if enabled.
        if ($item->getType() == 'uri') {
          if ($batch->fetch_labels) {
            $label = $item->getLabel();
            $label = $this->trimSample($label);
          }
          $value = $item->getValue();
        }
        else {
          $label = '';
          $value = $this->trimSample($item->getValue());
        }
        
        // Reduce duplicates by using hashes as keys
        // $key = md5(serialize($value));
        
        // $samples[$uri]['values'][$key]['type'] = $item->getType();
        // $samples[$uri]['values'][$key]['value'] = $value;
        // $samples[$uri]['values'][$key]['label'] = $label; 
        
        $samples[$uri]['values'][] = array (
          'type' => $item->getType(),
          'value' => $value,
          'label' => $label,
        );
        
        if (count($samples[$uri]['values']) == $max_samples) {
          // Strip out the hash keys
          // $samples[$uri]['values'] = $samples[$uri]['values'];
          continue;
        } 
      }
    }
    $batch->samples = $samples;
  }
  
  /**
   * Helper function to keep sample data short.
   *
   * @todo Use views_trim_text() here
   */
  public function trimSample($string, $max = 80, $delimeter = '...') { 
    
    // If the string is a URI just return it whole.
    if (rdfimporter_validate_uri($string)) return $string;
    
    $string = check_plain($string);
    if (strlen($string) > $max) { 
       // Try to trim cleanly.
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
   * Expose source form.
   */
  public function sourceForm($source_config) {
    drupal_add_css(drupal_get_path('module', 'rdfimporter') . '/rdfimporter.css');
    
    $form = array();
    $form['fetch_limit'] = array(
      '#type' => 'select',
      '#title' => t('Preview item limit'),
      '#description' => t('Maximum number of URIs to fetch for previewing.'),
      '#options' => drupal_map_assoc(array('no limit',1,2,3,4,5,10,15,20,50,100)),
      '#default_value' => !empty($source_config['fetch_limit']) ? $source_config['fetch_limit'] : RDFIMPORTER_PREVIEW_FETCH_LIMIT,
    );
    $form['sample_limit'] = array(
      '#type' => 'select',
      '#title' => t('Preview samples per property'),
      '#description' => t('Number of sample values to fetch for each different property.'),
      '#options' => drupal_map_assoc(array(1,2,3,4,5,6,7,8,9,10)),
      '#default_value' => !empty($source_config['sample_limit']) ? $source_config['sample_limit'] : RDFIMPORTER_MAX_SAMPLES,
    );
    $form['fetch_labels'] = array(
      '#type' => 'radios',
      '#title' => t('Fetch labels for preview'),
      '#description' => t('Attempt to fetch human-readable rdfs:labels for all object properties. This means fetching a lot of extra data, so enabling will make the preview <strong>significantly slower</strong>.'),
      '#options' => array(TRUE => t('Yes'), FALSE => t('No')),
      '#default_value' => !empty($source_config['fetch_labels']) ? $source_config['fetch_labels'] : 0,
    );
    
    // These three elements are for Feeds to use internally,
    // they are not displayed on any forms.
    $form['uri_list'] = array(
      '#type' => 'value',
      '#value' => !empty($source_config['uri_list']) ? $source_config['uri_list'] : '',
    );
    $form['samples'] = array(
      '#type' => 'value',
      '#value' => !empty($source_config['samples']) ? $source_config['samples'] : '',
    );
    $form['last_updated'] = array(
      '#type' => 'value',
      '#value' => !empty($source_config['last_updated']) ? $source_config['last_updated'] : '',
    );
    
    $samples = $this->getPreview($source_config);
    
    // Need to disable #tree structure here because Feeds will
    // use array_keys() on this form to get config variables. 
    $form['preview'] = array(
      '#type' => 'fieldset',
      '#title' => t('Preview'),
      '#collapsible' => TRUE,
      '#collapsed' => FALSE,
      '#attributes' => array('id' => 'rdfimporter-preview'),
      '#description' => ($samples) ? t('Last updated '. format_interval(time() - $source_config['last_updated'], 2) . ' ago') : '',
      '#tree' => FALSE,
      '#weight' => -5,
    );
    
    if ($samples) {
      $form['preview']['samples_table'] = array(
        '#prefix' => '<div>',
        '#value' => $samples,
        '#suffix' => '</div>',
      );
    }
    else {
      $form['preview']['status_message'] = array(
        '#prefix' => '<p>',
        '#value' => t('There is no preview saved for this feed yet.'),
        '#suffix' => '</p>',
      );
    }
    $form['refresh'] = array(
      '#type' => 'submit',
      '#name' => 'refresh',
      '#value' => t('Save and refresh preview'),
      '#submit' => array('rdfimporter_refresh_preview_submit'),
      '#executes_submit_callback' => TRUE,
    );
    $form['warning'] = array(
      '#prefix' => '<p><em>',
      '#value' => t('Note: Previewing often involves fetching a LOT of data and can take some time. To make things fast, turn labels OFF and use a low item limit setting.'),
      '#suffix' => '</em></p>',
    );
    
    // if ($samples) {
    //   $form['preview']['uri_list_display'] = array(
    //     '#type' => 'fieldset',
    //     '#title' => t('URIs'),
    //     '#description' => t('URIs that successfully returned data'),
    //     '#value' => theme_item_list($source_config['uri_list'], NULL, 'ol'),
    //     '#collapsible' => TRUE,
    //     '#collapsed' => TRUE,
    //   );
    // }
    return $form;
  }
  
  /**
   * Get a themed version of the feed samples to use as a preview.
   */
  public function getPreview($source_config) {
    if (is_array($source_config['samples']) && count($source_config['samples'])) {
      return theme_rdfimporter_samples($source_config['samples']);
    };
    return NULL;
  }
}
