<?php

/**
 * @file
 * Home of the LinkedDataImportFetcherResult.
 */

ld_import_include_arc();

/**
 * Class definition for LinkedDataImportFetcherResult.
 */
class LinkedDataImportFetcherResult extends FeedsFetcherResult {
  protected $uri_list;

  /**
   * Constructor.
   */
  public function __construct(array $uri_list) {
    // Always reset keys. Parser expects numerical keys beginning at 0.
    $this->uri_list = array_values($uri_list);
  }

  public function getList() {
    return $this->uri_list;
  }
}
