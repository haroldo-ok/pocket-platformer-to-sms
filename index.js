'use strict';

 const tmx = require('tmx-parser');
 
 tmx.parseFile('original/pocket-platformer.level-3.tmx', function(err, map) {
  if (err) throw err;
  console.log('map', map.layers[0].tiles);
});