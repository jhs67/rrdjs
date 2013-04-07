
var binding = require('./build/Release/rrdjs_bindings');

exports.create = function(fn, step, start, args, next) {
	var a = parseInt(start), b = parseInt(step);
	if (isNaN(a))  throw new Error("expected numeric start in rrdjs#create");
	if (isNaN(b) || b <= 0)  throw new Error("expected positive step in rrdjs#create");
	if (!Array.isArray(args)) throw new Error("expected args array in rrdjs#create");
	if (typeof next !== 'function') throw new Error("expected callback function in rrdjs#create");
	return binding.create(fn, b, a, args, next);
}

exports.info = function(fn, next) {
	if (typeof next !== 'function') throw new Error("expected callback function in rrdjs#update");
	return binding.info(fn, next);
}

exports.update = function(fn, tmpl, args, next) {
	if (!Array.isArray(args)) throw new Error("expected args array in rrdjs#update");
	if (typeof next !== 'function') throw new Error("expected callback function in rrdjs#update");
	return binding.update(fn, tmpl, args, next);
}

exports.fetch = function(fn, cf, start, end, step, next) {
	var a = parseInt(start), b = parseInt(end), c = parseInt(step);
	if (isNaN(a))  throw new Error("expected numeric start in rrdjs#fetch");
	if (isNaN(b))  throw new Error("expected numeric end in rrdjs#fetch");
	if (isNaN(c) || c <= 0)  throw new Error("expected positive start in rrdjs#fetch");
	if (typeof next !== 'function') throw new Error("expected callback function in rrdjs#fetch");
	return binding.fetch(fn, cf, a, b, c, next);
}
