<?php

/**
 * @file
 * Classes used by the Linked Data Import module.
 */

ld_import_include_arc();

class LdImportItem {

  protected $arc;
  protected $uri;
  protected $label;
  protected $language;

  public function __construct($uri, ARC2_Resource $arc = NULL, $language = NULL) {
    $this->uri = $uri;
    $this->arc = ($arc) ? $arc : ARC2::getResource();
    $this->language = $language;
    $this->label = ($values = $this->values(LD_IMPORT_LABEL_URI)) ? $values[0]['value'] : '';
  }

  public function __get($name) {
    return property_exists($this, $name) ? $this->$name : NULL;
  }

  public function hasData() {
    return ($this->arc->getProps(NULL, $this->uri)) ? TRUE : FALSE;
  }

  public function values($prop_uri) {
    $values = $this->arc->getProps($prop_uri, $this->uri);
    return ($values) ? $this->filterLanguage($values) : array();
  }

  protected function filterLanguage($items) {
    if ($this->language) {
      foreach ($items as $key => $item) {
        if (isset($item['lang']) && (strpos($item['lang'], $this->language) !== 0))  {
          unset($items[$key]);
        }
      }
    }
    return array_values($items);
  }
}
