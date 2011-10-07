<?php
// $Id$

/**
 * @file
 * Home of the LinkedDataImportFetcherResult.
 *
 * Throughout the processing stage, a single ARC_Resource object
 * gets passed around. This object handles all HTTP requests for
 * remote data and caches all the data it fetches. By passing around
 * this single instance, we can share all previously
 * fetched data and potentially save a whole lot of HTTP requests.
 */

/**
 * Class definition for LinkedDataImportFetcherResult.
 */
class LinkedDataImportFetcherResult extends FeedsFetcherResult {
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
    parent::__construct($uri_list);
    $this->uri_list = $uri_list;
    $this->arc = $arc;
  }

  /**
   * Implements FeedsFetcherResult::getRaw();
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
}
