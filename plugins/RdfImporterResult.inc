<?php
// $Id$

/**
 * @file
 * Home of the RdfImporterResult.
 *
 * Throughout the processing stage, a single ARC_Resource object
 * gets passed around. This object handles all HTTP requests for
 * remote data and caches all the data it fetches. By passing around
 * this single instance, we can share all previously
 * fetched data and potentially save a whole lot of HTTP requests. 
 */
 
/**
 * Class definition for RdfImporterResult.
 */
class RdfImporterResult extends FeedsFetcherResult {
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
    
    $this->createTempFile();
  }
  
  /**
   * Implementation of FeedsFetcherResult::getRaw();
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
   *
   * Triggers data fetching and only returns an individual if it has properties.
   */
  public function shiftItem() {
    $this->current_item = NULL;
    
    // Sometimes __wakeup() does not get called on unserialize()
    // and ARC is not automatically reloaded. Jump start it here...
    if (!isset($this->arc) || !is_object($this->arc)) {
      $this->loadArc();
    }
    
    // Move through the URI list until we hit a URI that returns data.
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
   * If a URI can't be fetched set a message.
   *
   * @todo It would be nice to consolidate this at the end of an import.
   */
  public function logErrors($uri, $errors = array()) {
    if (!empty($errors)) {
      $message = t('RDFimporter: The remote resource @uri was not imported. The following errors were returned: %errors', array('@uri' => $uri, '%errors' => implode('. ', $errors)));
      drupal_set_message($message, 'status');
    }
    else {
      $message = t('RDFimporter: The remote resource @uri was not imported. No data was returned.', array('@uri' => $uri));
      drupal_set_message($message, 'status');
    }
  }
  
  /**
   * Attempt to create a temp file on the filesystem for caching between batch operations.
   */
  public function createTempFile() {
    $this->temp_file = file_directory_temp() . '/rdfimporter_batch_' . md5(serialize($this->uri_list)) . '.tmp';
    if (!file_save_data(serialize($this->arc), $this->temp_file, FILE_EXISTS_REPLACE)) {
      drupal_set_message(t('RDFimporter: Could not write temporary files to the Drupal temp directory. Make sure the temp directory set at /admin/settings/file-system is writable.'), 'warning', FALSE);
    }
  }
  
  /**
   * Load a serialized ARC object from the temp file.
   *
   * If the ARC object can't be restored, just remove the property.
   * The import will continue to work, but previously-fetched data
   * will be re-fetched.
   */
  public function loadArc() {;
    if (file_exists($this->temp_file)) {
      $this->arc = @unserialize(file_get_contents($this->temp_file));
    }
    if (!is_object($this->arc)) {
      unset($this->arc);
    }
  }
  
  /**
   * Serialize the ARC object with all its data and save it to the temp file.
   */
  public function saveArc() {
    if (isset($this->arc)) {
      file_save_data(serialize($this->arc), $this->temp_file, FILE_EXISTS_REPLACE);
      unset($this->arc);
    }
  }
  
  /**
   * Empty out the current item before serializing so ARC data is not saved to the database.
   */
  public function clearItem() {
    $this->current_item = NULL;
  }
  
  /**
   * Make sure required ARC libraries are included after unserialize().
   *
   * Due to the way ARC2 includes libraries, necessary classes 
   * may not be available when a Feeds $batch gets serialized/unserialized.
   * Using __sleep() and __wakeup() we can ensure that everything
   * is loaded before reconstructing the ARC object.
   */
  function __wakeup() {
    // Make sure the ARC base is included.
    rdfimporter_include_arc();
    // Trigger additional file includes using ARC's helper functions.
    ARC2::getResource();
    ARC2::getRemoteStore();
    $this->loadArc();
  }
  
  function __sleep() {
    $this->saveArc();
    $this->clearItem();
    // MUST return something here.
    return array_keys(get_object_vars($this));
  }
  
}

