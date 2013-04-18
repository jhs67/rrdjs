rrdjs
=====

Minimal native bindings to the rrdtool libraries. The rrdtool source is embedded to minimize external dependencies.

## Usage

```javascript
var rrdjs = require('rrdjs');

var sources = [ "DS:test:GAUGE:120:U:U", "RRA:AVERAGE:0.5:1:129600" ];
rrdjs.create('test.rrd', 60, Date.now() / 1000, sources, created);

function created(err) {
  if (err) throw err;
  var time = Date.now() / 1000, sample = 100;
  rrdjs.update('test.rrd', 'test', [ time + ':' + sample ], updated);
}

function updated(err) {
  if (err) throw err;
  var time = Date.now() / 1000;
  rrdjs.fetch('test.rrd', 'AVERAGE', time - 300, time, 60, fetched)
}

function fetched(err, data) {
  if (err) throw err;
  console.log("Got some data", data);
}
````

## API

### `rrdjs.create(file, step, start, args, callback)`
Create an rrd file with the given step and start. The args array specifies the data sources and averages.

### `rrdjs.info(file, callback)`
Retreive information about the given rrd file.

### `rrdjs.update(file, ds, args, callback)`
Insert data into the rrd file.

### `rrdjs.fetch(file, start, end, step, callback)`
Retreive data from the rrd file.

## License

  MIT
