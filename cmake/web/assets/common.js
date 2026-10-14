const ipinfo_endpoint = 'https://hypersomnia.io/geolocation';

function decodeCrazyGamesToken(token) {
  // A JWT's payload is base64url (- and _, no padding) of UTF-8 JSON - atob alone throws on it or garbles names
  const base64url = token.split('.')[1];
  const base64 = base64url.replace(/-/g, '+').replace(/_/g, '/') + '='.repeat((4 - base64url.length % 4) % 4);
  const payload = JSON.parse(new TextDecoder().decode(Uint8Array.from(atob(base64), c => c.charCodeAt(0))));
  console.log("Decoded token payload", payload);

  // How long it still lives: by this computer's clock when that looks right (a cached token is not fresh),
  // else by the token's own lifetime - this computer's clock may be off by hours
  const currentTime = Math.floor(Date.now() / 1000);
  const lifetime = typeof payload.iat === 'number' ? payload.exp - payload.iat : null;
  const left = payload.exp - currentTime;
  payload.expiresIn = lifetime === null || (left > 0 && left <= lifetime) ? left : lifetime;
  console.log(currentTime, payload.exp);

  return payload;
}

// At most 3 s - the sign-in waits for it, and a join started meanwhile would go in as a guest
function downloadAvatar(avatarUrl, callback) {
  // An AbortController with a timer - AbortSignal.timeout is missing in older browsers (Safari < 16)
  const controller = typeof AbortController !== 'undefined' ? new AbortController() : null;

  if (controller) {
    setTimeout(() => controller.abort(), 3000);
  }

  fetch(avatarUrl, controller ? { signal: controller.signal } : {})
    .then(response => response.arrayBuffer())
    .then(buffer => {
      const byteArray = new Uint8Array(buffer);
      callback(byteArray);
    })
    .catch(error => {
      console.error('Error downloading avatar:', error);
      callback(new Uint8Array());
    });
}

function passAuthDataToCpp(provider, profileId, profileName, avatarUrl, authToken, expiresIn) {
  downloadAvatar(avatarUrl, (avatarByteArray) => {
    Module.ccall('on_auth_data_received', 'void', 
      ['string', 'string', 'string', 'array', 'number', 'string', 'number'],
      [provider, profileId, profileName, avatarByteArray, avatarByteArray.length, authToken, expiresIn]
    );
  });
}

function getUserGeolocation() {
  fetch(ipinfo_endpoint)
    .then(response => response.json())
    .then(data => {
      const [latitude, longitude] = data.loc.split(',');
      console.log("User's geolocation: ", latitude, longitude);
      Module.ccall('on_geolocation_received', 'void', ['double', 'double'], [parseFloat(latitude), parseFloat(longitude)]);
    })
    .catch(error => {
      console.error("Error getting geolocation: ", error);
    });
}

// Module.hs_tab_blocked: the last tab did not open (e.g. Safari's popup blocker, a click too long ago) - the game says so.
// The game opens it a frame after the click, which a strict browser (Safari) does not count as the click's -
// a blocked one is opened again from inside the next click itself (e.g. on "Open it again").
function tryOpenTab(urlStr) {
  try {
    const opened = !!window.open(urlStr, '_blank');

    if (opened) {
      Module.hs_last_opened_url = urlStr;
      Module.hs_last_opened_at = Date.now();
    }

    return opened;
  } catch (e) {
    console.warn("Could not open URL (may be blocked by iframe restrictions or popup blocker):", e);
    return false;
  }
}

function openUrl(url) {
  const urlStr = UTF8ToString(url);

  // Just opened by the click itself (below) - the game's own call for the same click opens no second tab
  if (Module.hs_last_opened_url === urlStr && Date.now() - Module.hs_last_opened_at < 2000) {
    Module.hs_tab_blocked = false;
    return;
  }

  Module.hs_tab_blocked = !tryOpenTab(urlStr);

  if (Module.hs_tab_blocked) {
    Module.hs_blocked_url = urlStr;

    document.addEventListener('pointerup', () => {
      if (Module.hs_blocked_url === urlStr) {
        Module.hs_blocked_url = null;
        Module.hs_tab_blocked = !tryOpenTab(urlStr);
      }
    }, { once: true, capture: true });
  }
}

function hideProgress() {
  const elementsToRemove = [
    'progress-container'
  ];

  elementsToRemove.forEach(id => {
    const element = document.getElementById(id);
    if (element) {
      element.remove(); // Remove the element from the DOM
    }
  });

  document.body.style.backgroundImage = 'none';
}

function setBrowserLocation(newLocation) {
  const locStr = UTF8ToString(newLocation);

  console.log("New location: ", locStr);
  try {
    window.history.pushState("object or string", "Title", locStr);
  } catch (e) {
    console.warn("Could not update browser location (possibly in a cross-origin iframe):", e);
  }
}

function setBrowserLocation_cg(newLocation) {
  if (window.CrazyGames) {
    const locStr = UTF8ToString(newLocation);

    if (locStr === "") {
      window.CrazyGames.SDK.game.hideInviteButton();
    }
    else {
      const inviteLink = window.CrazyGames.SDK.game.inviteLink({ game: locStr });
      console.log("Invite link", inviteLink);

      const link = window.CrazyGames.SDK.game.showInviteButton({ game: locStr });
      console.log("Invite button link", link);
    }
  }
}

const timeZoneMap = {
  'au': 'Australia/Sydney',
  'ru': 'Europe/Moscow',
  'de': 'Europe/Berlin',
  'us-central': 'America/Chicago',
  'pl': 'Europe/Warsaw',
  'ch': 'Europe/Zurich'
};

function getTimeZoneName(locationId) {
  return timeZoneMap[locationId] || '';
}

function get_secs_until_next_weekend_evening(locationId) {
  const timeZoneName = getTimeZoneName(locationId);
  if (!timeZoneName) {
    return -1; // Invalid locationId
  }

  try {
    const now = new Date();
    const formatter = new Intl.DateTimeFormat('en-US', {
      timeZone: timeZoneName,
      year: 'numeric',
      month: '2-digit',
      day: '2-digit',
      hour: '2-digit',
      minute: '2-digit',
      second: '2-digit',
      hour12: false
    });

    // Format current date-time in the given time zone
    const parts = formatter.formatToParts(now).reduce((acc, part) => {
      acc[part.type] = part.value;
      return acc;
    }, {});

    const nowInZone = new Date(`${parts.year}-${parts.month}-${parts.day}T${parts.hour}:${parts.minute}:${parts.second}`);

    const weekendEvenings = [
      new Date(nowInZone.getFullYear(), nowInZone.getMonth(), nowInZone.getDate() + (5 - nowInZone.getDay()), 19, 0, 0, 0), // Friday 19:00
      new Date(nowInZone.getFullYear(), nowInZone.getMonth(), nowInZone.getDate() + (6 - nowInZone.getDay()), 19, 0, 0, 0), // Saturday 19:00
      new Date(nowInZone.getFullYear(), nowInZone.getMonth(), nowInZone.getDate() + (7 - nowInZone.getDay()), 19, 0, 0, 0)  // Sunday 19:00
    ];

    let closestDistance = Infinity;

    for (let evening of weekendEvenings) {
      if (evening < nowInZone) {
	evening.setDate(evening.getDate() + 7); // Move to next week
      }

      const durationUntilEvening = (evening - nowInZone) / 1000; // Difference in seconds

      if (durationUntilEvening < closestDistance) {
	closestDistance = durationUntilEvening;
      }

      if (durationUntilEvening <= 2 * 60 * 60 && durationUntilEvening >= 0) {
	return 0.0; // Ongoing event
      }
    }

    return closestDistance;
  } catch (error) {
    console.error(error);
    return -1;
  }
}

function set_status(text) {
  console.log("set_status: " + text);

  var m = text.match(/([^(]+)\((\d+(\.\d+)?)\/(\d+)\)/);
  var statusElement = document.getElementById('downloading-text');
  var progressElement = document.getElementById('progress-bar-inner');
  var progressMbElement = document.getElementById('progress-mb');

  if (!statusElement || !progressElement || !progressMbElement) {
    console.log("Couldn't set_status: element is null.");
    return;
  }

  if (m) {
    var currentBytes = parseFloat(m[2]);
    var totalBytes = parseFloat(m[4]);
    var progress = currentBytes / totalBytes * 100;

    var currentMB = (currentBytes / (1024 * 1024)).toFixed(2);
    var totalMB = (totalBytes / (1024 * 1024)).toFixed(2);

    statusElement.innerHTML = 'Downloading... ' + progress.toFixed(0) + '%';
    progressElement.style.width = progress + '%';
    progressMbElement.innerHTML = `${currentMB} MB of ${totalMB} MB`; // Update MB progress text
    document.getElementById('spinner').style.display = 'block';
  } else {
    statusElement.innerHTML = text;
  }

  if (text) {
    document.getElementById('spinner').style.display = 'block';
    progressMbElement.style.display = 'block'; // Ensure MB progress text is visible during download
  } else {
    statusElement.innerHTML = 'Entering the game world.';
    progressElement.style.width = '100%';
  }
}

function monitor_run_dependencies(left) {
  this.totalDependencies = Math.max(this.totalDependencies, left);
  var progress = ((this.totalDependencies - left) / this.totalDependencies) * 100;
  document.getElementById('progress-bar-inner').style.width = progress + '%';
  document.getElementById('downloading-text').innerHTML = 'Downloading... ' + progress.toFixed(0) + '%';
}

function resizeCanvas() {
  var canvas = document.getElementById('canvas');
  const dpr = window.devicePixelRatio || 1;

  const {width, height} = canvas.getBoundingClientRect();
  const displayWidth = Math.round(width * dpr);
  const displayHeight = Math.round(height * dpr);

  canvas.width = displayWidth;
  canvas.height = displayHeight;
}

const user_config_path = 'user/runtime_prefs.json';

function init_config_cg() {
  const fileContent = window.CrazyGames.SDK.data.getItem('config.json');
  if (fileContent) {
    FS.writeFile(user_config_path, fileContent, { encoding: 'utf8' });
  }
  else {
    console.warn('config.json has not yet been created. Nothing to load.');
  }
}

function save_config_cg() {
  if (window.CrazyGames) {
    if (FS.analyzePath(user_config_path).exists) {
      const fileContent = FS.readFile(user_config_path, { encoding: 'utf8' });
      window.CrazyGames.SDK.data.setItem('config.json', fileContent);
    }
    else {
      console.error(`File ${user_config_path} does not exist.`);
    }
  }
}

async function pre_run_cg() {
  Module.addRunDependency('cginit');
  console.log("pre_run_cg");

  if (window.CrazyGames) {
    try {
      await window.CrazyGames.SDK.init();

      FS.mkdir('/user');
      init_config_cg();

      {
        const invite_link = window.CrazyGames.SDK.game.getInviteParam("game");

        if (invite_link) {
          Module.cg_invite_link = invite_link;
          console.log("invite link: ", invite_link);
        }
        else {
          Module.cg_invite_link = "";
          console.log("no invite link");
        }
      }

      const available = window.CrazyGames.SDK.user.isUserAccountAvailable;
      console.log("User account system available", available);

      // Signed in on the portal itself while the game runs: the game gets the token at once, no reload
      if (available && window.CrazyGames.SDK.user.addAuthListener) {
        window.CrazyGames.SDK.user.addAuthListener(user => {
          if (user && Module.ccall) {
            window.CrazyGames.SDK.user.getUserToken().then(passCrazyGamesToken).catch(e => console.log("Token retrieval failed:", e));
          }
        });
      }

      if (available) {
        try {
          const token = await window.CrazyGames.SDK.user.getUserToken();
          console.log("Got a CrazyGames token.");

          const payload = decodeCrazyGamesToken(token);

          Module.initial_user = {
            userId: payload.userId,
            username: payload.username,
            profilePictureUrl: payload.profilePictureUrl,
            token: token,
            expiresIn: payload.expiresIn
          };
        } catch (e) {
          console.log("getUserToken failed:", e);
        }
      }
    } catch (e) {
      console.log("CG SDK init error: ", e);
    }
  } else {
    console.log("window.CrazyGames is undefined!");
  }

  console.log("pre_run_cg finished");
  Module.removeRunDependency('cginit');
}

function try_fetch_initial_user() {
  if (Module.initial_user) {
    const u = Module.initial_user;
    passAuthDataToCpp('crazygames', u.userId, u.username, u.profilePictureUrl, u.token, u.expiresIn);

    return true;
  }

  return false;
}

function request_fresh_auth_token() {
  if (window.CrazyGames) {
    window.CrazyGames.SDK.user.getUserToken()
      .then(token => {
        const payload = decodeCrazyGamesToken(token);

        passAuthDataToCpp('crazygames', payload.userId, payload.username, payload.profilePictureUrl, token, payload.expiresIn);
        return true;
      })
      .catch(e => {
        console.log("Failed to refresh auth token:", e);
        return false;
      });

    return true;
  }
  else {
    console.log("window.CrazyGames is undefined!");
    return false;
  }
}

// A line over the game when what it saves can not reach this browser's storage
function show_storage_notice(text) {
  let notice = document.getElementById('storage-notice');

  if (!notice) {
    notice = document.createElement('div');
    notice.id = 'storage-notice';
    notice.style.cssText = 'position:fixed;top:0;left:0;right:0;z-index:1000;padding:6px 10px;background:#a02020;color:#fff;font:14px sans-serif;text-align:center;pointer-events:none';
    document.body.appendChild(notice);
  }

  notice.textContent = text;
}

// Ctrl/Cmd+V is left to the browser: it then fires its "paste" event with the system clipboard, which the game types
// into the focused field. Reaching the game, the shortcut would be stopped there and paste the game's own older copy.
window.addEventListener('keydown', e => {
  if ((e.ctrlKey || e.metaKey) && (e.key === 'v' || e.key === 'V')) {
    e.stopImmediatePropagation();
  }
}, true);

function pre_run() {
  // One tab writes the account at a time - two would each write their own copy over the other's (a new account, a link)
  if (navigator.locks && navigator.locks.request) {
    navigator.locks.request('hypersomnia_user', { ifAvailable: true }, lock => {
      if (!lock) {
        Module.idbfs_readonly = true;
        show_storage_notice('The game is open in another tab - nothing is saved in this one. Close it here and play in the other tab.');
        return;
      }

      // Held while this tab lives
      return new Promise(() => {});
    }).catch(() => {});
  }

  // Add a run dependency to ensure syncing is done before the application starts
  Module.addRunDependency('idbfs');

  // The account key lives in this browser's storage: ask the browser not to clear it under storage pressure
  // Safari clears a site's storage after 7 days without a visit unless it is persisted - the game warns there (Info).
  // Other browsers refusing persist() only clear it under storage pressure - no warning for them.
  // Every browser on iOS is WebKit underneath - the same rules
  // (an iPad says "Macintosh" - told apart by its touch screen)
  const is_safari = /^((?!chrome|chromium|android|crios|fxios|edg).)*safari/i.test(navigator.userAgent)
    || /iPad|iPhone|iPod/.test(navigator.userAgent)
    || (/Macintosh/.test(navigator.userAgent) && navigator.maxTouchPoints > 1);

  if (navigator.storage && navigator.storage.persist) {
    navigator.storage.persist().then(granted => {
      console.log("Persistent storage:", granted);
      Module.hs_storage_may_be_cleared = is_safari && !granted;
    }).catch(() => {
      Module.hs_storage_may_be_cleared = is_safari;
    });
  }
  else {
    Module.hs_storage_may_be_cleared = is_safari;
  }

  // Create a folder inside our virtual file system
  FS.mkdir('/user');
  // Mount IDBFS on this folder
  FS.mount(IDBFS, {}, '/user');

  // The account key apart from everything else (maps, caches): its own IndexedDB database, so a full or failed write
  // of the rest never takes the key with it, and a name no other game's "/user" shares
  FS.mkdir('/hypersomnia_keys');
  FS.mount(IDBFS, {}, '/hypersomnia_keys');

  // Synchronize from IndexedDB to memory
  FS.syncfs(true, function (err) {
    if (err) {
      // Nothing is ever written back then: an empty start would make a new account key and wipe the stored one
      console.error('Error loading from IndexedDB', err);
      Module.idbfs_failed = true;
      // Embedded on another site (or in private mode) the browser may keep no storage at all - reloading does not help there
      const embedded = (() => { try { return window.self !== window.top; } catch (e) { return true; } })();
      show_storage_notice(embedded
        ? "This browser keeps no storage for the game here (embedded on another site, or in private mode) - nothing is saved. Play at play.hypersomnia.io to keep your account."
        : "Couldn't read this browser's storage - your account was not loaded and nothing is saved. Reload the page.");
    }
    else console.log('Loaded from IndexedDB');

    // Remove the run dependency after IDBFS is fully loaded
    Module.removeRunDependency('idbfs');
  });
}

function get_cli_args(for_cg) {
  var urlPath = window.location.pathname;
  var queryString = window.location.search;

  console.log("urlPath:", urlPath);
  console.log("queryString:", queryString);

  var urlParams = new URLSearchParams(queryString);
  var instantJoin = urlParams.get('instantJoin');

  if (instantJoin === 'true') {
    return "/host";
  }

  return urlPath + queryString;
}

function sync_idbfs() {
  if (Module.idbfs_failed || Module.idbfs_readonly) {
    console.log("Not writing to the browser's storage (it did not load, or another tab writes it).");
    return;
  }

  if (Module.isSyncing) {
    Module.needsSync = true; // Mark that a new sync is needed after the current one
    console.log("Still syncing, delaying syncfs");
    return;
  }

  Module.isSyncing = true;
  Module.needsSync = false;

  console.log("Calling FS.syncfs.");

  FS.syncfs(false, function(err) {
    Module.isSyncing = false;
    if (err) {
      // Tried again later, a few times - not in a tight loop (e.g. the quota is full)
      console.error("Filesystem sync failed", err);
      Module.sync_failures = (Module.sync_failures || 0) + 1;

      if (Module.sync_failures <= 5) {
        setTimeout(() => Module.sync_idbfs(), 1000 * Math.min(30, Math.pow(2, Module.sync_failures)));
      }
      else {
        show_storage_notice("Your progress can't be saved in this browser (its storage is full or blocked). Back up your private key.");
      }
    } else {
      Module.sync_failures = 0;
      console.log("Filesystem synced successfully");
      if (Module.needsSync) {
	console.log("Resyncing.");
	Module.sync_idbfs(); // There was a request during sync, so we sync again
      }
    }
  });
}

function sync_idbfs_cg() {
  save_config_cg();
}

function passCrazyGamesToken(token) {
  const payload = decodeCrazyGamesToken(token);
  passAuthDataToCpp('crazygames', payload.userId, payload.username, payload.profilePictureUrl, token, payload.expiresIn);
}

function loginCrazyGames() {
  if (window.CrazyGames) {
    window.CrazyGames.SDK.user.showAuthPrompt()
      .then(user => {
        if (user) {
          return window.CrazyGames.SDK.user.getUserToken();
        } else {
          throw new Error('User cancelled login');
        }
      })
      .then(passCrazyGamesToken)
      .catch(e => {
        // Signed in on the portal already (e.g. in another game): the prompt refuses, the token is there
        if (e && (e.code === 'userAlreadySignedIn' || String(e.message || e).includes('userAlreadySignedIn'))) {
          window.CrazyGames.SDK.user.getUserToken().then(passCrazyGamesToken).catch(e2 => console.log("Token retrieval failed:", e2));
          return;
        }

        console.log("Auth prompt or token retrieval failed:", e);
      });
  }
  else {
    console.log("window.CrazyGames is undefined!");
  }
}

function sdk_happy_time() {
  if (window.CrazyGames) {
    console.log("Crazygames: happytime");
    window.CrazyGames.SDK.game.happytime();
  }
  else {
    console.log("happytime (no SDK)");
  }
}

function sdk_gameplay_start() {
  if (window.CrazyGames) {
    console.log("Crazygames: gameplayStart");
    window.CrazyGames.SDK.game.gameplayStart();
  }
}

function sdk_gameplay_stop() {
  if (window.CrazyGames) {
    console.log("Crazygames: gameplayStop");
    window.CrazyGames.SDK.game.gameplayStop();
  }
}

function sdk_loading_start() {
  if (window.CrazyGames) {
    console.log("Crazygames: loadingStart");
    window.CrazyGames.SDK.game.loadingStart();
  }
}

function sdk_loading_stop() {
  if (window.CrazyGames) {
    console.log("Crazygames: loadingStop");
    window.CrazyGames.SDK.game.loadingStop();
  }
}

function sdk_request_ad() {
  if (window.CrazyGames) {
    const callbacks = {
      adFinished: () => {
        console.log("End midgame ad");
        Module.ccall('on_ad_ended', 'void', [], []);
      },
      adError: (error) => {
        console.log("Error midgame ad", error);
        Module.ccall('on_ad_ended', 'void', [], []);
      },
      adStarted: () => {
        console.log("Start midgame ad");
        Module.ccall('on_ad_started', 'void', [], []);
      },
    };

    window.CrazyGames.SDK.ad.requestAd("midgame", callbacks);

    return true;
  }

  return false;
}

function create_module(for_cg) {
  const for_io = !for_cg;

  var Module = {
    canvas: (function() {
      var canvas = document.getElementById('canvas');
      resizeCanvas();
      return canvas;
    })(),
    arguments: [get_cli_args(for_cg)],
    preRun: [],
    postRun: [],
    setStatus: set_status,
    totalDependencies: 0,
    monitorRunDependencies: monitor_run_dependencies,
    cg_invite_link: ""
  };

  Module.getUserGeolocation = getUserGeolocation;

  Module.try_fetch_initial_user = try_fetch_initial_user;
  Module.request_fresh_auth_token = request_fresh_auth_token;

  if (for_cg) {
    Module.sync_idbfs = sync_idbfs_cg;
    Module['preRun'].push(pre_run_cg);

    Module.loginCrazyGames = loginCrazyGames;
    Module.setBrowserLocation = setBrowserLocation_cg;
  }

  if (for_io) {
    Module.sync_idbfs = sync_idbfs;
    Module['preRun'].push(pre_run);

    Module.setBrowserLocation = setBrowserLocation;
  }

  window.addEventListener('resize', resizeCanvas);
  window.addEventListener("keydown", (e) => {
    if (e.code === "F11" || e.code === "F12") {
      e.preventDefault = function() { console.log("ignore prevent default"); };
    }
  }, { capture: true });

  Module.setStatus('Downloading...');

  window.onerror = function() {
    if (document.getElementById('progress-container')) {
      Module.setStatus('Exception thrown, see JavaScript console');
      document.getElementById('spinner').style.display = 'none';
    }
  };

  Module.sdk_happy_time = sdk_happy_time;
  Module.sdk_gameplay_start = sdk_gameplay_start;
  Module.sdk_gameplay_stop = sdk_gameplay_stop;
  Module.sdk_loading_start = sdk_loading_start;
  Module.sdk_loading_stop = sdk_loading_stop;
  Module.sdk_request_ad = sdk_request_ad;

  return Module;
}
