<?php

/**
 * @file
 * Class definition for LinkedDataImportResource.
 *
 * This class is mainly a wrapper around the functionality provided by
 * an ARC2_Resource object.
 *
 * @see ARC2_Resource
 */

ld_import_include_arc();

/**
 * Class definition for LinkedDataImportResource.
 */
class LinkedDataImportResource {
  protected $uri;
  protected $label;
  protected $language;
  protected $arc;

  public function __construct($uri, ARC2_Resource $arc, $language = NULL) {
    $this->uri = $uri;
    $this->arc = $arc;
    $this->language = $language;
  }

  public function getUri() {
    return $this->uri;
  }

  public function getLabel() {
    if (!isset($this->label)) {
      $this->label = $this->getPropValue('http://www.w3.org/2000/01/rdf-schema#label');
    }
    return $this->label;
  }

  public function getLanguage() {
    return $this->language;
  }

  public function getArc() {
    return $this->arc;
  }

  public function getProps() {
    $props = $this->arc->getProps(NULL, $this->uri);
    foreach ($props as &$prop) {
      $prop = $this->filterLanguage($prop);
    }
    return $props;
  }

  public function getProp($prop_uri) {
    $prop = $this->arc->getProps($prop_uri, $this->uri);
    return $this->filterLanguage($prop);
  }

  public function getPropValues($prop_uri) {
    $values = array();
    if ($items = $this->arc->getProps($prop_uri, $this->uri)) {
      foreach ($this->filterLanguage($items) as $item) {
        $values[] = $item['value'];
      }
    }
    return $values;
  }

  public function getPropValue($prop_uri) {
    $values = $this->getPropValues($prop_uri);
    return !empty($values) ? reset($values) : '';
  }

  public function hasData() {
    $props = $this->arc->getProps(null, $this->uri);
    return count($props) ? TRUE : FALSE;
  }

  /**
   * If the language is explicitly set for property values, strip out any
   * values that do not match the target language.
   *
   * It's up to the calling function to decide what to return if nothing's left.
   *
   * Typically the site language should be a two-letter code like: en, fr, es
   * but the value's language could have extensions like: en-US, en-UK
   * so we try to match as a substring instead of an exact match.
   *
   * @param $items
   *   An ARC2 array of property values
   * @return
   *   A filtered array of ARC2 property values
   */
  protected function filterLanguage($items) {
    if ($this->language) {
      foreach ($items as $key => $item) {
        if (isset($item['lang']) && (strpos($item['lang'], $this->language) !== 0))  {
          unset($items[$key]);
        }
      }
    }
    return $items;
  }
}