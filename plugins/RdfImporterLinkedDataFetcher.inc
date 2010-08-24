<?php
// $Id$

/**
 * @file
 * Home of the RdfImporterLinkedDataFetcher.
 */
 
// The RdfImporterBatch class has been separated so it can
// be used with multiple fetchers.
require_once(dirname(__FILE__) .'/RdfImporterBatch.inc');

/**
 * Build a list of URIs to fetch and fire up a batch.
 */
class RdfImporterLinkedDataFetcher extends FeedsFetcher {
  
  /**
   * Implementation of FeedsFetcher::fetch().
   */
  public function fetch(FeedsSource $source) {
    $source_config = $source->getConfigFor($this);

    $uri_list = $this->getURIs($source_config);
    $arc = ARC2::getResource();
    
    $preview = ($source->preview) ? TRUE : FALSE;
    
    return new RdfImporterBatch($uri_list, $arc, $preview);
  }
  
  /**
   * Get a list of URIs from the source config
   */
  public function getURIs($source_config) {
    $uri_list = array();

    // Gather manually-entered URIs
    if (!empty($source_config['list'])) {
      $items = explode("\n", $source_config['list']);
      if (!empty($items)) {
        foreach ($items as $v) {
          $v = trim($v);
          if (!empty($v) && rdfimporter_validate_uri($v)) {
            $uri_list[$v] = $v;
          }
        }
      }
    }
    
    return array_values($uri_list);
  }
  
  /**
   * Expose source form.
   *
   * @todo Add a good decription for this form
   */
  public function sourceForm($source_config) {
    $form = array();
    $form['list'] = array(
      '#type' => 'textarea',
      '#title' => t('URI list'),
      '#description' => t('Enter a list of URIs to be imported. Place each URI on a separate line.'),
      '#default_value' => isset($source_config['list']) ? $source_config['list'] : '',
      '#rows' => 8,
      '#required' => TRUE,
    );
    return $form;
  }
  
  /**
   * Override parent::sourceFormValidate().
   * 
   * @todo Invalid form elements are not getting highlighted
   */
  public function sourceFormValidate(&$values) {
    // Validate the list of URIs
    if (!empty($values['list'])) {
      $items = explode("\n", $values['list']);
      if (!empty($items)) {
        foreach ($items as $v) {
          $v = trim($v);
          if (!empty($v) && !rdfimporter_validate_uri($v)) {
            form_set_error('feeds][list', t('The URI "%v" is not valid.', array('%v' => $v)));
          }
        }
      }
      else {
        form_set_error('feeds][list', t('There is a problem with the list of URIs entered. Make sure they are valid and each is on a separate line.'));
      }
    }
  }
}