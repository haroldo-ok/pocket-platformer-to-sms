'use strict';

 const tmx = require('tmx-parser');
 
 const arrayOfSize = n => Array(n).fill();
 
 tmx.parseFile('original/pocket-platformer.level-3.tmx', function(err, map) {
	if (err) throw err;

	const { width, height } = map;
	
	// The map will be converted into a columnar format, to make the horizontal scrolling faster
	
	const layer = map.layers[0];
	const columns = arrayOfSize(height).map((_, x) => arrayOfSize(width).map((_, y) => {
		const tile = layer.tileAt(x, y);
		return tile ? tile.gid : 0;
	}));

	console.log({ width, height, columns });	  
});