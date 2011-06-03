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
  public function __construct() {
    parent::__construct();
  }
  
  /**
   * Implementation of FeedsImportBatch::getRaw();
   *
   * @todo Once batching is supported at the fetching/parsing level
   *  it would be nice to attempt fetching data here. But for now
   *  we have to wait until the processing stage.
   */
  public function getRaw() {
    return "";
  }

}

