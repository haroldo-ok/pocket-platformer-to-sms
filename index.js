'use strict';

 const tmx = require('tmx-parser');
 
 const arrayOfSize = n => Array(n).fill();
 
 tmx.parseFile('original/pocket-platformer.level-3.tmx', function(err, map) {
	if (err) throw err;

	const { width, height } = map;
	
	const layer = map.layers[0];
	const columns = arrayOfSize(height).map((_, y) => arrayOfSize(width).map((_, x) => {
		const tile = layer.tileAt(x, y);
		return tile ? tile.gid : 0;
	}));

	console.log({ width, height, columns });	  
});