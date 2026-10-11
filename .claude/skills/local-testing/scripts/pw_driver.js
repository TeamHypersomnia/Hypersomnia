/*
	A Chromium (Playwright) kept open and driven over HTTP, for clicking through the web build step by step:
	  NODE_PATH=<dir with playwright> node pw_driver.js <url> <control port> [cookie name=value@domain]
	  curl "localhost:<port>/shot?f=/path.png"   screenshot of the current page
	  curl "localhost:<port>/click?x=..&y=.."     /type?t=..   /key?k=Escape   /wheel?x=..&y=..&dy=..
	  curl "localhost:<port>/pages"               urls of the open tabs; /page?i=N switches to one
	  curl "localhost:<port>/console"             the console lines so far     /quit
	Sound muted, WebGL through SwiftShader. Headed - run it under Xvfb.
*/
const http = require('http');
const { chromium } = require('playwright');

(async () => {
	const [url, port, cookie] = process.argv.slice(2);
	const browser = await chromium.launch({
		headless: false,
		args: ['--use-angle=swiftshader', '--enable-unsafe-swiftshader', '--mute-audio', '--autoplay-policy=no-user-gesture-required']
	});
	const context = await browser.newContext({ viewport: { width: 1280, height: 720 } });

	if (cookie) {
		const [nv, domain] = cookie.split('@');
		const eq = nv.indexOf('=');
		await context.addCookies([{ name: nv.slice(0, eq), value: nv.slice(eq + 1), domain, path: '/' }]);
	}

	const lines = [];
	let page = await context.newPage();

	const watch = p => p.on('console', m => lines.push(`[${p.url().slice(0, 60)}] ${m.text()}`));
	watch(page);
	context.on('page', p => watch(p));

	await page.goto(url);

	http.createServer(async (req, res) => {
		const u = new URL(req.url, 'http://x');
		const q = k => u.searchParams.get(k);
		try {
			switch (u.pathname) {
				case '/shot': await page.screenshot({ path: q('f') }); break;
				case '/click': await page.mouse.move(Number(q('x')), Number(q('y'))); await page.waitForTimeout(100); await page.mouse.click(Number(q('x')), Number(q('y')), { delay: 120 }); break;
				case '/type': await page.keyboard.type(q('t'), { delay: 50 }); break;
				case '/key': await page.keyboard.press(q('k')); break;
				case '/wheel': await page.mouse.move(Number(q('x')), Number(q('y'))); await page.mouse.wheel(0, Number(q('dy'))); break;
				case '/pages': return res.end(context.pages().map((p, i) => `${i} ${p.url()}`).join('\n') + '\n');
				case '/page': page = context.pages()[Number(q('i'))]; await page.bringToFront(); break;
				case '/console': return res.end(lines.join('\n') + '\n');
				case '/quit': res.end('bye\n'); await browser.close(); process.exit(0);
			}
			res.end('ok\n');
		} catch (e) {
			res.end('error: ' + e.message + '\n');
		}
	}).listen(Number(port), '127.0.0.1');
})();
