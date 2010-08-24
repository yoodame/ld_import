<?php
// $Id$

/**
 * @file
 * Home of the RdfImporterBatch.
 *
 * To support batching during fetching, Feeds must be patched 
 * @see http://drupal.org/node/744660
 *
 * Troughout the import process, a single ARC_Resource object
 * gets passed around. This object handles all HTTP requests for
 * remote data and caches all the data it fetches. By passing around
 * this single instance, we can share all previously
 * fetched data and save a whole lot of HTTP requests. 
 */
 
define('RDFIMPORTER_FETCHER_BATCH_SIZE', 5);

/**
 * Definition of the import batch object created on the fetching stage by RDF Importer fetchers
 */
class RdfImporterBatch extends FeedsImportBatch {
  protected $uri_list;
  protected $arc;
  public $fetcher_position;
  public $fetcher_total;
  public $fetcher_batch_size;
  public $samples;
  public $preview;
  
  /**
   * Constructor.
   */
  public function __construct($uri_list, ARC2_Resource $resource, $preview = FALSE) {
    parent::__construct();
    $this->uri_list = $uri_list;
    $this->fetcher_position = 0;
    $this->fetcher_total = count($uri_list);
    
    // Only use batch limit if batching is supported
    // and if this is not a standalone import
    if (method_exists($this, 'getProgress')) {
      $this->setTotal(FEEDS_PARSING, count($uri_list));
      $this->fetcher_batch_size = RDFIMPORTER_FETCHER_BATCH_SIZE;
    }
    else {
      $this->fetcher_batch_size = 1000000;
    }
    
    $this->arc = $resource;
    $this->preview = $preview;
  }
  
  /**
   * Provide read access to properties with magic method __get().
   */
  public function __get($name) {
    if ($this->$name) {
      return $this->$name;
    }
    return NULL;
  }
  
  /**
   * Implementation of FeedsImportBatch::getRaw();
   *
   * @todo It might be nice to retry failed fetching 
   *   attempts instead of just logging them.
   * @todo Consolidate some of the error logging
   *   and provide a final status message if some
   *   resources could not be fetched
   */
  public function getRaw() {
    // $this->arc->index = array();
    
    $position = $this->fetcher_position;
    $total = $this->fetcher_total;
    $max = $this->fetcher_batch_size;
    
    $i = 0;
    $individuals = array();
    
    while ($position < $total && $i < $max) {
      $uri = $this->uri_list[$position];
      $this->arc->resetErrors();
      $this->arc->setURI($uri);
      $props = $this->arc->getProps();
      
      // Only continue importing this URI if there were no errors
      // and property data was retrieved
      if (!($errors = $this->arc->getErrors()) && !empty($props)) {
        $individuals[$uri] = $uri;
      }
      elseif (!$errors && empty($props)) {
        watchdog('rdfimporter', 'The remote resource @uri could not be fetched. No data was returned.', array('@uri' => '<' . $uri . '>'));
      }
      else {
        watchdog('rdfimporter', 'The remote resource @uri could not be fetched. The following errors were returned: %errors', array('@uri' => '<' . $uri . '>', '%errors' => implode('. ', $errors)));
      }
      
      $i++;
      $position++;
    }
    
    $this->fetcher_position = $position;
    
    // Only set progress if batching is supported
    if (method_exists($this, 'getProgress')) {
      $progress = ($position == $total) ? FEEDS_BATCH_COMPLETE : $position;
      $this->setProgress(FEEDS_PARSING, $progress);
    }
    
    return $individuals;
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
    // Trigger file includes using ARC's helper functions
    ARC2::getResource();
    ARC2::getRemoteStore();
    $this->arc = unserialize($this->arc);
  }
}