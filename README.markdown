RDFimporter module
==================
RDFimporter is primarily a set of plugins for the [Feeds module][feeds] that can be used to import RDF content from remote sources and map that content to Drupal nodes. It can be used for both one-time imports and periodic data imports.

Most of the Drupal functionality comes from the Feeds module itself. All RDF fetching and parsing is handled by the [ARC2 RDF framework][arc2].

This module results from work being done at Cornell University's [Mann Library][mann] to bring content from Cornell's [VIVO database][vivo] into Drupal. We hope this code will be useful to others as RDF becomes more available on the Web. 

[feeds]: http://drupal.org/project/feeds
[arc2]: http://arc.semsol.org/
[mann]: http://mannlib.cornell.edu
[vivo]: http://vivo.cornell.edu


Goals
=====
- Make it easy for site builders, including non-developers, to import remote RDF content
- Allow RDF to be mapped to standard Drupal structures (CCK, Taxonomy)
- Provide a way to update nodes manually or automatically


Features
========
- Support for Linked Data and SPARQL endpoints (via ARC2)
- Allows users to define mappings between RDF properties and Drupal node properties
- Requires very little knowledge about RDF and SPARQL
- Automatically fetches human-readable labels for object properties
- Provides hooks to 'preprocess' retrieved data and hooks to expose additional node properties


Requirements
============
- **Drupal 6.x**
- **Feeds module** + dependencies
- **ARC2 PHP library**

**NOTE:** Some system settings for PHP and MySQL may also need to be changed depending on how much data is being imported. See the INSTALL file for more details.


Installation and usage
======================
See "INSTALL.markdown" for an overview of how to configure and use the module.


Documentation
=============
Links to further documentation and screencasts will be added here as they become available.


Support
=======
This code is definitely a work in progress. All comments, questions and suggestions are welcome. 


Other modules
=============
There are some great modules available that provide similar functionality, including [SPARQL Views][sparql_views] and [Linked Data][linked_data]. Compared to those modules, RDFimporter does things quite lazily. It requires very little familiarity with RDF or SPARQL, but this ultimately makes it inefficient in many cases. If the data you're after is available from a SPARQL endpoint and you can write SPARQL queries, you might be better off with something like [SPARQL Views + Feeds View Parser][sparql_views_screencasts].

[sparql_views]: http://drupal.org/project/sparql_views
[linked_data]: http://drupal.org/project/linked_data
[sparql_views_screencasts]: http://lin-clark.com/blog/importing-syncing-content-external-sites-wikipedia

