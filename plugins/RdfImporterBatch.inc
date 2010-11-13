<?php
// $Id$

/**
 * @file
 * Home of the RdfImporterBatch.
 *
 * Troughout the processing stage, a single ARC_Resource object
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
  
  // These 
  public $samples;
  public $sample_limit;
  public $language;
  
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
      $this->arc->resetErrors();
      $this->arc->setURI($uri);
      // $props = $this->arc->getProps();
      
      // Only continue importing this URI if there were no errors
      // and property data was retrieved
      // if (!($errors = $this->arc->getErrors()) && !empty($props)) {
      if (!($errors = $this->arc->getErrors())) {
        $individuals[$uri] = $uri;
      }
      // elseif (!$errors && empty($props)) {
      //   watchdog('rdfimporter', 'The remote resource @uri could not be fetched. No data was returned.', array('@uri' => $uri));
      // }
      // else {
      //   watchdog('rdfimporter', 'The remote resource @uri could not be fetched. The following errors were returned: %errors', array('@uri' => $uri, '%errors' => implode('. ', $errors)));
      // }
    }
    return $individuals;
  }
  
  /**
   * Override FeedsImportBatch:shiftItem() to return RdfImporterIndividual objects.
   */
  public function shiftItem() {
    $next = array_shift($this->items);
    $this->current_item = NULL;
    if ($next) {
      $individual = new RdfImporterIndividual($next, $this->arc, $this->language);
      $this->current_item = ($individual->getProperties()) ? $individual : NULL;
    }
    return $this->current_item;
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
    $this->arc = serialize($this->arc);
    return array_keys(get_object_vars($this));
  }
  
  function __wakeup() {
    rdfimporter_include_arc();
    // Trigger file includes using ARC's helper functions.
    ARC2::getResource();
    ARC2::getRemoteStore();
    $this->arc = unserialize($this->arc);
  }
}

