<?php

/**
 * @file
 * Class definitions for Linked Data Import's fetcher plugins.
 */

/**
 * Class definition for LdImportFetcher.
 */
class LdImportFetcher extends FeedsFetcher {

  /**
   * Implements FeedsFetcher::fetch().
   */
  public function fetch(FeedsSource $source) {
    return new LdImportFetcherResult($this->getURIs($source));
  }

  /**
   * Gets the list of URIs entered by the user.
   */
  public function getURIs($source) {
    $source_config = $source->getConfigFor($this);
    $uri_list = array();

    if (empty($source_config['list']) || (!$items = explode("\n", $source_config['list']))) {
      return $uri_list;
    }

    // Gather entered URIs.
    foreach ($items as $v) {
      $v = trim($v);
      if ($v && $this->validURI($v)) {
        $uri_list[$v] = $v;
      }
    }
    return array_values($uri_list);
  }

  /**
   * Validates a URI string.
   *
   * @todo Double check this - regexes were taken from the D6 RDF module.
   */
  public function validURI($uri) {
    return preg_match("/^([a-z]+):\/\/[a-z0-9\/:_\-_\.\?\$,;~=#&%\+]+$/i", $uri) || preg_match('/^urn:/', $uri) || preg_match('/^mailto:/', $uri);
  }

  /**
   * Override parent::sourceForm().
   */
  public function sourceForm($source_config) {
    $form['list'] = array(
      '#type' => 'textarea',
      '#title' => t('URI List'),
      '#description' => t('Enter a list of URIs to be imported. Place each URI on a separate line.'),
      '#default_value' => isset($source_config['list']) ? $source_config['list'] : '',
      '#rows' => 15,
    );
    return $form;
  }

  /**
   * Override parent::sourceForm().
   */
  public function sourceFormValidate(&$values) {
    if (empty($values['list'])) {
      return;
    }
    // Validate the list of URIs.
    if ($items = explode("\n", $values['list'])) {
      foreach ($items as $v) {
        $v = trim($v);
        if ($v && !$this->validURI($v)) {
          form_set_error('feeds][list', t('The URI "%v" is not valid.', array('%v' => $v)));
        }
      }
    }
    else {
      form_set_error('feeds][list', t('There is a problem with the list of URIs entered. Make sure they are valid and each is on a separate line.'));
    }
  }
}

/**
 * Class definition for LdImportFetcherResult.
 */
class LdImportFetcherResult extends FeedsFetcherResult {

  public function __construct(array $uri_list) {
    $this->uri_list = $uri_list;
  }

  public function getList() {
    return $this->uri_list;
  }
}
