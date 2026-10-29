(function () {
	'use strict';

	var MAX_SHOWN = 200000;
	var HINTS = {
		GET: 'Fetches the URL. Error pages are shown too, and the body can be opened in a new window.',
		POST: 'Sends the body to the URL. The target needs a location that allows POST (for example /uploads/). A new file is stored under the URL path.',
		DELETE: 'Deletes the file at the URL. The URL is required and the request is sent as soon as the button is pressed.'
	};

	var $ = function (id) { return document.getElementById(id); };
	var form = $('form'), methodEl = $('method'), urlEl = $('url');
	var bodyBox = $('bodyBox'), textBox = $('textBox'), fileBox = $('fileBox');
	var bodyText = $('bodyText'), bodyFile = $('bodyFile');
	var hint = $('hint'), errorEl = $('error'), sendBtn = $('send');
	var result = $('result'), badge = $('badge'), verdict = $('verdict');
	var meta = $('meta'), raw = $('raw');
	var openBtn = $('openBody'), copyBtn = $('copyRaw');

	var urlEdited = false;
	var current = null;
	var blobUrl = null;

	function bodyKind() {
		return form.querySelector('input[name="bodyKind"]:checked').value;
	}

	function showError(message) {
		errorEl.textContent = message;
		errorEl.hidden = !message;
	}

	function syncMethod() {
		var method = methodEl.value;
		var isPost = method === 'POST';

		bodyBox.hidden = !isPost;
		hint.textContent = HINTS[method];
		hint.className = method === 'DELETE' ? 'hint warn' : 'hint';
		sendBtn.textContent = 'Send ' + method;
		sendBtn.className = method === 'DELETE' ? 'danger' : '';
		urlEl.placeholder = method === 'GET' ? '/' : '/s/file.txt';
		if (!urlEdited && method === 'GET')
			urlEl.value = '/';
		else if (!urlEdited)
			urlEl.value = '';
		showError('');
	}

	function syncBodyKind() {
		var isFile = bodyKind() === 'file';

		textBox.hidden = isFile;
		fileBox.hidden = !isFile;
	}

	function normalizeUrl(value) {
		var url = value.trim();

		if (url === '')
			return '';
		if (/^[a-z][a-z0-9+.-]*:\/\//i.test(url))
			return url;
		return url.charAt(0) === '/' ? url : '/' + url;
	}

	function isTextual(type) {
		return type === '' || /^text\//i.test(type) || /(json|xml|javascript|x-www-form-urlencoded)/i.test(type);
	}

	function statusClass(status) {
		if (status >= 200 && status < 300) return 'ok';
		if (status >= 300 && status < 400) return 'redirect';
		if (status >= 400 && status < 500) return 'client';
		return 'server';
	}

	function describe(method, status, text) {
		var what = method + ' ' + status + ' ' + text;

		if (status >= 200 && status < 300)
			return method + ' succeeded';
		if (status >= 300 && status < 400)
			return method + ' answered with a redirect';
		if (status >= 400 && status < 500)
			return method + ' was refused (client error)';
		return what + ': the server reported an error';
	}

	function render(info) {
		result.hidden = false;
		badge.className = 'badge ' + info.cls;
		badge.textContent = info.badge;
		verdict.textContent = info.verdict;
		meta.textContent = info.meta;
		raw.textContent = info.raw;
		openBtn.disabled = !info.hasBody;
		openBtn.textContent = info.hasBody ? 'Open body in a new window' : 'No body to open';
		copyBtn.textContent = 'Copy raw response';
		current = info;
		result.scrollIntoView({ behavior: 'smooth', block: 'nearest' });
	}

	function renderFailure(method, url, error) {
		render({
			cls: 'none',
			badge: 'no response',
			verdict: method + ' got no answer',
			meta: method + ' ' + url,
			raw: 'The request failed before a response arrived (' + error + ').\n\n' +
				'The server may have closed the connection, for example right after refusing a large body ' +
				'(the browser reports that as a network error), or it is not running.',
			hasBody: false,
			blob: null
		});
	}

	function buildRaw(res, buffer, textual) {
		var lines = ['HTTP/1.1 ' + res.status + ' ' + res.statusText];
		var body = '';

		res.headers.forEach(function (value, name) {
			lines.push(name + ': ' + value);
		});
		if (buffer.byteLength === 0)
			body = '';
		else if (textual) {
			body = new TextDecoder('utf-8').decode(buffer);
			if (body.length > MAX_SHOWN)
				body = body.slice(0, MAX_SHOWN) + '\n\n[display cut after ' + MAX_SHOWN + ' characters, the whole body opens in the new window]';
		}
		else
			body = '[binary body, ' + buffer.byteLength + ' bytes: open it in the new window]';
		return lines.join('\n') + '\n\n' + body;
	}

	function send(method, url, init) {
		var started = performance.now();

		sendBtn.disabled = true;
		init.method = method;
		init.cache = 'no-store';
		fetch(url, init).then(function (res) {
			return res.arrayBuffer().then(function (buffer) {
				var type = res.headers.get('content-type') || '';
				var ms = Math.round(performance.now() - started);
				var where = res.redirected ? '  (redirected to ' + res.url.replace(location.origin, '') + ')' : '';

				if (blobUrl)
					URL.revokeObjectURL(blobUrl);
				blobUrl = buffer.byteLength ? URL.createObjectURL(new Blob([buffer], { type: type || 'text/plain' })) : null;
				render({
					cls: statusClass(res.status),
					badge: res.status + ' ' + res.statusText,
					verdict: describe(method, res.status, res.statusText),
					meta: method + ' ' + url + where + '  -  ' + ms + ' ms  -  ' + buffer.byteLength + ' body bytes',
					raw: buildRaw(res, buffer, isTextual(type)),
					hasBody: buffer.byteLength > 0,
					blob: blobUrl
				});
			});
		}).catch(function (error) {
			renderFailure(method, url, error && error.message ? error.message : 'unknown error');
		}).then(function () {
			sendBtn.disabled = false;
		});
	}

	form.addEventListener('submit', function (event) {
		var method = methodEl.value;
		var url = normalizeUrl(urlEl.value);
		var init = {};

		event.preventDefault();
		showError('');
		if (url === '') {
			showError(method === 'DELETE' ? 'Enter the URL of the file to delete, for example /uploads/file.txt.' :
				method === 'POST' ? 'Enter the URL to post to, for example /uploads/file.txt.' : 'Enter a URL.');
			urlEl.focus();
			return;
		}
		urlEl.value = url;
		if (method === 'POST') {
			if (bodyKind() === 'file') {
				if (!bodyFile.files.length) {
					showError('Choose a file to uploads.');
					return;
				}
				init.body = bodyFile.files[0];
				init.headers = { 'Content-Type': bodyFile.files[0].type || 'application/octet-stream' };
			}
			else {
				init.body = bodyText.value;
				init.headers = { 'Content-Type': 'text/plain' };
			}
		}
		send(method, url, init);
	});

	methodEl.addEventListener('change', syncMethod);
	form.querySelectorAll('input[name="bodyKind"]').forEach(function (radio) {
		radio.addEventListener('change', syncBodyKind);
	});
	urlEl.addEventListener('input', function () { urlEdited = true; });

	bodyFile.addEventListener('change', function () {
		var file = bodyFile.files[0];

		if (file && (!urlEdited || urlEl.value.trim() === '')) {
			urlEl.value = '/uploads/' + encodeURIComponent(file.name);
			urlEdited = false;
		}
	});

	openBtn.addEventListener('click', function () {
		if (current && current.blob)
			window.open(current.blob, '_blank');
	});

	copyBtn.addEventListener('click', function () {
		var done = function (ok) { copyBtn.textContent = ok ? 'Copied' : 'Copy failed'; };

		try {
			navigator.clipboard.writeText(raw.textContent).then(function () { done(true); }, function () { done(false); });
		} catch (e) {
			done(false);
		}
	});

	syncMethod();
	syncBodyKind();
}());
