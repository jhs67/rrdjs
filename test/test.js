
var assert = require('assert');
var rrdjs = require('../index');

describe('rrdjs', function() {
	describe('create', function() {
		it('should work', function(done) {
			var args = [ "DS:test:GAUGE:120:U:U", "RRA:AVERAGE:0.5:1:129600", "RRA:AVERAGE:0.25:5:57600" ];
			var start = Math.ceil(Date.now() / 60 / 1000 - 2) * 60;
			rrdjs.create("test.rrd", 60, start, args, done);
		})
	})
	describe('info', function() {
		it('should work', function(done) {
			rrdjs.info("test.rrd", done);
		})
	})
	describe('update', function() {
		it('should work', function(done) {
			var start = Math.ceil(Date.now() / 60 / 1000 - 1) * 60;
			rrdjs.update("test.rrd", "test", [ start + ":" + 100 ], done);
		})
	})
	describe('fetch', function() {
		it('should work', function(done) {
			var start = Math.ceil(Date.now() / 60 / 1000 - 1) * 60;
			rrdjs.fetch("test.rrd", "AVERAGE", start - 180, start, 60, done);
		})
	})
	describe('info again', function() {
		it('should work', function(done) {
			rrdjs.info("test.rrd", done);
		})
	})
	describe('clean up', function() {
		it('should work', function(done) {
			require('fs').unlink('test.rrd', done);
		})
	})
})
