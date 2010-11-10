RDFimporter module
==================
RDFimporter is primarily a set of plugins for the [Feeds module][1] that can be used to import RDF content from remote sources and map that content to Drupal nodes. It can be used for both one-time imports and periodic data imports.

Most of the Drupal functionality comes from the Feeds module itself. All RDF fetching and parsing is handled by the [ARC2 RDF framework][2].

[1]: http://drupal.org/project/feeds
[2]: http://arc.semsol.org/


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
- **Feeds module** 
- **ARC2 PHP library**

**NOTE:** Some default settings for PHP and MySQL may also need to be changed depending on how much data is being imported. See the INSTALL file for more details.


Installation and usage
======================
See "INSTALL.markdown" for an overview of how to configure and use the module.


Documentation
=============
Links to further documentation and screencasts will be added here as they become available.


Background
==========
This module resulted work being done at Cornell University's [Mann Library][5] to bring content from Cornell's [VIVO database][6] into Drupal. We hope this code will be useful to others as RDF becomes more available on the Web. 

[5]: http://mannlib.cornell.edu
[6]: http://vivo.cornell.edu


Support
=======
This code is definitely a work in progress. All comments, questions and suggestions are welcome. 


Other modules
=============
There are some great modules available that provide similar functionality, including [SPARQL Views][7] and [Linked Data][8]. Compared to those modules, RDFimporter does things quite lazily. It requires very little familiarity with RDF or SPARQL, but this ultimately makes it inefficient in many cases. If the data you're after is available from a SPARQL endpoint and you can write SPARQL queries, you might be better off with something like [SPARQL Views + Feeds View Parser][9].

[7]: http://drupal.org/project/sparql_views
[8]: http://drupal.org/project/linked_data
[9]: http://lin-clark.com/blog/importing-syncing-content-external-sites-wikipedia

