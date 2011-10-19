<?php

/**
 * @file
 * Home of the LinkedDataImportFetcher.
 */

require_once(dirname(__FILE__) .'/LinkedDataImportFetcherResult.inc');

/**
 * Class definition for LinkedDataImportFetcher.
 */
class LinkedDataImportFetcher extends FeedsFetcher {

  /**
   * Implementation of FeedsFetcher::fetch().
   */
  public function fetch(FeedsSource $source) {
    return new LinkedDataImportFetcherResult($this->getURIs($source));
  }

  /**
   * Get the list of URIs entered by the user.
   */
  public function getURIs($source) {
    $uri_list = array();
    $source_config = $source->getConfigFor($this);

    // Gather entered URIs.
    if (!empty($source_config['list'])) {
      $items = explode("\n", $source_config['list']);
      if (!empty($items)) {
        foreach ($items as $v) {
          $v = trim($v);
          if (!empty($v) && ld_import_validate_uri($v)) {
            // Reduce duplicates.
            $uri_list[$v] = $v;
          }
        }
      }
    }
    return array_values($uri_list);
  }
  
  public function getArc() {
    return ARC2::getResource();
  }

  /**
   * Expose source form.
   */
  public function sourceForm($source_config) {
    $form = array();
    $form['list'] = array(
      '#type' => 'textarea',
      '#title' => t('URI list'),
      '#description' => t('Enter a list of URIs to be imported. Place each URI on a separate line.'),
      '#default_value' => isset($source_config['list']) ? $source_config['list'] : '',
      '#rows' => 15,
      '#required' => TRUE,
    );
    return $form;
  }

  /**
   * Override parent::sourceFormValidate().
   */
  public function sourceFormValidate(&$values) {
    // Validate the list of URIs.
    if (!empty($values['list'])) {
      $items = explode("\n", $values['list']);
      if (!empty($items)) {
        foreach ($items as $v) {
          $v = trim($v);
          if (!empty($v) && !ld_import_validate_uri($v)) {
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
