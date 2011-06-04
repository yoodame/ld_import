<?php
// $Id$

/**
 * @file
 * Definitions for RdfImporterResource, RdfImporterIndividual, 
 * RdfImporterProperty, and RdfImporterPropertyValue classes.
 *
 * These classes are mainly wrappers around the functionality
 * provided by an ARC2_Resource object
 * @see ARC2_Resource
 */
 
define('RDFS_LABEL_URI', 'http://www.w3.org/2000/01/rdf-schema#label');
 
/**
 * Class definition for RdfImporterResource.
 */
class RdfImporterResource {
  protected $uri;
  protected $label;
  protected $language;
  protected $arc;
  
  public function __construct($uri = NULL, $arc = NULL, $site_language = NULL) {
    $this->uri = $uri;
    $this->label = '';
    $this->language = ($site_language) ? $site_language : 'en';
    if ($arc instanceof ARC2_Resource) {
      $this->arc = $arc;
    }
    else {
      $this->setArc();
    }
  }
  
  public function __get($name) {
    if ($this->$name) {
      return $this->$name;
    }
    return NULL;
  }
  
  public function setArc() {
    rdfimporter_include_arc();
    $this->arc = ARC2::getResource();
  }
  
  public function clearArc() {
    if (isset($this->arc->index[$this->uri])) {
      unset($this->arc->index[$this->uri]);
    }
  }
  
  public function setEndpoint($endpoint) {
    $endpoint = array('remote_store_endpoint' => $endpoint);
    $this->arc->setStore(ARC2::getRemoteStore($endpoint));
  }
  
  public function getUri() {
    return $this->uri;
  }
  
  public function setUri($uri) {
    $this->uri = $uri;
  }
  
  public function getLabel() {
    if (empty($this->label)) {
      $this->setLabel();
    }
    return $this->label;
  }
  
  public function setLabel() {
    $this->label = $this->getPropValue(RDFS_LABEL_URI);
  }
  
  /**
   * When grabbing a single value from a property containing
   * multiple values, try to grab a value that matches 
   * the target language. If not, just use the first value.
   */
  public function getPropValue($prop_uri) {
    $this->arc->setURI($this->uri);
    $items = $this->arc->getProps($prop_uri);
    $item = reset($items);
    switch ($count = count($items)) {
      case ($count == 1):
        return $item['value'];
      case ($count > 1):
        // Try to find a value in the target language, otherwise just return the first value.
        $filtered = $this->filterLanguage($items);
        $item = ($filtered) ? reset($filtered) : $item;
        return $item['value'];
      default:
        return '';
    }
  }
  
  public function getPropValues($prop_uri) {
    $this->arc->setURI($this->uri);
    $items = $this->arc->getProps($prop_uri);
    $values = array();
    if ($filtered = $this->filterLanguage($items)) {
      foreach ($filtered as $item) {
        $values[] = $item['value'];
      }
    }
    return $values;
  }

  /**
   * If the language is specified in a set of property values, strip out any
   * values that do not match the site language.
   *
   * It's up to the calling function to decide what to return if nothing's left.
   *
   * The site language should be a two-letter code like: en, fr, es
   * but the value's language could have extensions like: en-US, en-UK
   * so we try to match as a substring instead of an exact match.
   * 
   * @param $items
   *   An array of ARC2 property values
   * @return
   *   An array of property values either in the target language or with no language specified
   */
  protected function filterLanguage($items) {
    foreach ($items as $uri => $item) {
      if (isset($item['lang']) && (strpos($item['lang'], $this->language) !== 0))  {
        unset($items[$uri]);
      }
    }
    return count($items) ? $items : array();
  }
}

/**
 * Class definition for RdfImporterIndividual.
 */
class RdfImporterIndividual extends RdfImporterResource {
  protected $properties;
  
  public function __construct($uri = NULL, $arc = NULL, $site_language = NULL) {
    parent::__construct($uri, $arc, $site_language);
  }
  
  public function getProperty($prop_uri) {
    $props = $this->getProperties();
    return isset($props[$prop_uri]) ? $props[$prop_uri] : array();
  }
  
  public function hasProperties() {
    if (!isset($this->properties)) {
      $this->setProperties();
    }
    return count($this->properties) ? TRUE : FALSE;
  }
  
  public function getProperties() {
    if (!isset($this->properties)) {
      $this->setProperties();
    }
    return $this->properties;
  }
  
  public function setProperties() {
    $this->arc->setURI($this->uri);
    $props = $this->arc->getProps();
    if (!empty($props)) {
      foreach ($props as $uri => $prop) {
        $prop = new RdfImporterProperty($uri, $this->uri, $this->arc, $this->language);
      }
    }
    $this->properties = $props;
  }
  
  /**
   * Grab the first value for each property and offer as a sample.
   */
  public function getSamples() {
    $samples = array();
    $this->arc->setURI($this->uri);
    $props = $this->arc->getProps();
    foreach ($props as $uri => $values) {
      // Try to find a property value in the target language, otherwise just use the first one
      $item = ($filtered = $this->filterLanguage($values)) ? $filtered[array_rand($filtered)] : $values[array_rand($values)];
      $samples[$uri] = new RdfImporterPropertyValue($item, $this->arc, $this->language);
    }
    return $samples;
  }
}

/**
 * Class definition for RdfImporterProperty.
 */
class RdfImporterProperty extends RdfImporterResource {
  protected $individual_uri;
  protected $values;
  
  public function __construct($property_uri, $individual_uri, $arc = NULL, $site_language = NULL) {
    parent::__construct($property_uri, $arc, $site_language);
    
    // Set the individual (subject) URI
    $this->individual_uri = $individual_uri;
    
    // Instead of fetching a label by default, just make one from the URI
    // This can be overridden by using $this->setLabel().
    $this->label = $this->arc->extractTermLabel($this->uri);
  }
  
  public function getValue($filter_language = TRUE) {
    if (!isset($this->values)) {
      $this->setValues($filter_language);
    }
    return reset($this->values);
  }
  
  public function getValues($filter_language = TRUE) {
    if (!isset($this->values)) {
      $this->setValues($filter_language);
    }
    return $this->values;
  }
  
  public function setValues($filter_language = TRUE) {
    $values = $this->arc->getProps($this->uri, $this->individual_uri);
    if ($filter_language) {
      $values = ($filtered = $this->filterLanguage($values)) ? $filtered : array();
    }
    foreach ($values as $uri => $value) {
      $values[$uri] = new RdfImporterPropertyValue($value, $this->arc, $this->language);
    }
    $this->values = $values;
  }
}

/**
 * Class definition for RdfImporterPropertyValue.
 */
class RdfImporterPropertyValue extends RdfImporterResource {
  protected $item;
  
  public function __construct($item, $arc = NULL, $site_language = NULL) {
    parent::__construct(NULL, $arc, $site_language);
    $this->item = $item;
    
    // If this is an object property, set the active URI. Otherwise make sure it's NULL.
    // If the URI is NULL, arc will return empty arrays for methods that attempt
    // to fetch properties. This would return empty arrays when attempting to get
    // a label for a data property
    $this->uri = ($this->getType() == 'uri') ? $this->getValue() : NULL;
  }
  
  public function getValue() {
    return $this->item['value'];
  }
  
  public function getType() {
    return $this->item['type'];
  }
  
  public function getLanguage() {
    return $this->item['lang'];
  }
}