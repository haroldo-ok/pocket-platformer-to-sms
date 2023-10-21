'use strict';

const fs = require('fs');

const tmx = require('tmx-parser');

const arrayOfSize = n => Array(n).fill();

const findMapLayerOfType = (map, type) => map.layers.find(layer => layer.type == type);
 
tmx.parseFile('original/pocket-platformer.level-3.tmx', function(err, map) {
	if (err) throw err;

	const { width, height } = map;
	
	// The map will be converted into a columnar format, to make the horizontal scrolling faster
	
	const layer = findMapLayerOfType(map, 'tile');
	const columns = arrayOfSize(width).map((_, x) => arrayOfSize(height).map((_, y) => {
		const tile = layer.tileAt(x, y);
		return tile ? tile.gid : 0;
	}));
	
	console.log(findMapLayerOfType(map, 'object').objects);
		
	const buffer = Buffer.from([width, height, ...columns.flat()], 'application/octet-stream');
	fs.writeFileSync('experiment/data/map3.bin', buffer, 'binary');
});