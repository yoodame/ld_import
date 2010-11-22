<?php
// $Id$

/**
 * @file
 * Home of the RdfImporterBatch.
 *
 * Throughout the processing stage, a single ARC_Resource object
 * gets passed around. This object handles all HTTP requests for
 * remote data and caches all the data it fetches. By passing around
 * this single instance, we can share all previously
 * fetched data and potentially save a whole lot of HTTP requests. 
 */
 
/**
 * Class definition for RdfImporterBatch.
 */
class RdfImporterBatch extends FeedsImportBatch {
  public $uri_list;
  public $arc;
  public $samples;
  
  public $sample_limit;
  public $fetch_labels;
  public $language;
  public $temp_file;
  
  /**
   * Constructor.
   */
  public function __construct($uri_list, ARC2_Resource $arc) {
    parent::__construct();
    $this->uri_list = $uri_list;
    $this->arc = $arc;
    $this->samples = array();
    $this->sample_limit = RDFIMPORTER_MAX_SAMPLES;
    $this->fetch_labels = FALSE;
    
    // Get the current site language to limit content
    // to that language where possible.
    global $language;
    $this->language = $language->language;
    
    $this->temp_file = file_directory_temp() . '/rdfimporter_batch_' . md5(serialize($uri_list)) . '.temp';
    $this->createTempFile();
  }
  
  /**
   * Implementation of FeedsImportBatch::getRaw();
   *
   * @todo Once batching is supported at the fetching/parsing level
   *  it would be nice to attempt fetching data here. But for now
   *  we have to wait until the processing stage.
   */
  public function getRaw() {
    $individuals = array();
    foreach ($this->uri_list as $uri) {
      $individuals[$uri] = $uri;
    }
    return $individuals;
  }
  
  /**
   * Override FeedsImportBatch:shiftItem() to return RdfImporterIndividual objects.
   *
   * Triggers data fetching and only returns an individual if it has properties.
   */
  public function shiftItem() {
    $this->current_item = NULL;
    while ($uri = array_shift($this->items)) {
      $individual = new RdfImporterIndividual($uri, $this->arc, $this->language);
      $individual->arc->resetErrors();
      if ($individual->hasProperties()) {
        $this->current_item = $individual;
        break;
      }
      else {
        $this->logErrors($uri, $individual->arc->getErrors());
      }
    }
    return $this->current_item;
  }
  
  /**
   * If a URI can't be fetched add it to the log and set a message.
   *
   * @todo It would be nice to consolidate this at the end of an import.
   */
  public function logErrors($uri, $errors = array()) {
    if (!empty($errors)) {
      $message = t('The remote resource @uri was not imported. The following errors were returned: %errors', array('@uri' => $uri, '%errors' => implode('. ', $errors)));
      drupal_set_message($message, 'status');
      // watchdog('rdfimporter', $message);
    }
    else {
      $message = t('The remote resource @uri was not imported. No data was returned.', array('@uri' => $uri));
      drupal_set_message($message, 'status');
      // watchdog('rdfimporter', $message);
    }
  }
  
  public function createTempFile() {
    if (!file_save_data(serialize($this->arc), $this->temp_file, FILE_EXISTS_REPLACE)) {
      throw new Exception(t('RDFimporter could not write temporary files to the Drupal temp directory. Make sure the temp directory set at /admin/settings/file-system is writable.'));
    }
  }
  
  public function deleteTempFile() {
    file_delete($this->temp_file);
  }
  
  public function loadArc() {
    $arc = @unserialize(file_get_contents($this->temp_file));
    if (is_object($arc)) {
      $this->arc = $arc;
    }
    else {
      throw new Exception(t('RDFimporter encountered a problem loading data from the filesystem.'));
    }
  }
  
  public function saveArc() {;
    file_save_data(serialize($this->arc), $this->temp_file, FILE_EXISTS_REPLACE);
  }
  

  

  
  
  /**
   * Make sure required ARC libraries are included after unserialize()
   *
   * Due to the way ARC2 includes libraries, necessary classes 
   * may not be available when a Feeds $batch gets serialized/unserialized, 
   * making $this->arc unusable. Using __sleep() and __wakeup() we can
   * be sure that everything is loaded before reconstructing $this->arc.
   */
  function __sleep() {
    $this->saveArc();
    unset($this->arc);
    return array_keys(get_object_vars($this));
  }
  
  function __wakeup() {
    // Make sure the ARC base is included.
    rdfimporter_include_arc();
    // Trigger additional file includes using ARC's helper functions.
    ARC2::getResource();
    ARC2::getRemoteStore();
    $this->loadArc();
  }
  
  function __destruct() {
    $this->deleteTempFile();
  }
}

