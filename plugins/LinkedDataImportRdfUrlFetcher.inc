<?php

/**
 * @file
 * Home of the LinkedDataImportRdfUrlFetcher.
 */

require_once(dirname(__FILE__) . '/LinkedDataImportFetcherResult.inc');

/**
 * Class definition for LinkedDataImportRdfUrlFetcher.
 */
class LinkedDataImportRdfUrlFetcher extends LinkedDataImportFetcher {

  /**
   * Get the list of URIs entered by the user.
   */
  public function getURIs(FeedsSource $source) {
    $source_config = $source->getConfigFor($this);

    $arc = ARC2::getRDFParser();
    $arc->parse($source_config['url']);

    $uri_list = array();
    foreach ($arc->getSimpleIndex() as $uri => $data) {
      $uri_list[] = $uri;
    }
    return $uri_list;
  }

  /**
   * Expose source form.
   */
  public function sourceForm($source_config) {
    $form = array();
    $form['url'] = array(
      '#type' => 'textfield',
      '#title' => t('RDF URL'),
      '#description' => t('Enter the URL for an RDF document. All subjects will be imported.'),
      '#default_value' => isset($source_config['url']) ? $source_config['url'] : '',
      '#maxlength' => NULL,
      '#required' => TRUE,
    );
    return $form;
  }

  /**
   * Override parent::sourceFormValidate().
   */
  public function sourceFormValidate(&$values) {
    if (!feeds_valid_url($values['url'], TRUE)) {
      $form_key = 'feeds][' . get_class($this) . '][url';
      form_set_error($form_key, t('The URL %url is invalid.', array('%url' => $values['url'])));
    }
  }
}
