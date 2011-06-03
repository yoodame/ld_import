<?php
// $Id$

/**
 * @file
 * Home of the RdfImporterLinkedDataFetcher.
 */

// The RdfImporterResult class has been separated so it can
// be used with multiple fetchers.
require_once(dirname(__FILE__) .'/RdfImporterResult.inc');

/**
 * Class definition for RdfImporterLinkedDataFetcher.
 */
class RdfImporterLinkedDataFetcher extends FeedsFetcher {

  /**
   * Implementation of FeedsFetcher::fetch().
   */
  public function fetch(FeedsSource $source) {
    return new RdfImporterResult($this->getURIs($source), ARC2::getResource());
  }

  /**
   * Get the list of URIs entered by the user.
   */
  public function getURIs($source) {
    $source_config = $source->getConfigFor($this);
    $uri_list = array();

    // Gather entered URIs.
    if (!empty($source_config['list'])) {
      $items = explode("\n", $source_config['list']);
      if (!empty($items)) {
        foreach ($items as $v) {
          $v = trim($v);
          if (!empty($v) && rdfimporter_validate_uri($v)) {
            // Reduce duplicates.
            $uri_list[$v] = $v;
          }
        }
      }
    }
    return array_values($uri_list);
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
   *
   * @todo Invalid form elements are not being highlighted
   */
  public function sourceFormValidate(&$values) {
    // Validate the list of URIs.
    if (!empty($values['list'])) {
      $items = explode("\n", $values['list']);
      if (!empty($items)) {
        foreach ($items as $v) {
          $v = trim($v);
          if (!empty($v) && !rdfimporter_validate_uri($v)) {
            form_set_error('feeds][RdfImporterLinkedDataFetcher][list', t('The URI "%v" is not valid.', array('%v' => $v)));
          }
        }
      }
      else {
        form_set_error('feeds][RdfImporterLinkedDataFetcher][list', t('There is a problem with the list of URIs entered. Make sure they are valid and each is on a separate line.'));
      }
    }
  }
}