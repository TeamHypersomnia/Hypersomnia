const ipinfo_endpoint = 'https://hypersomnia.xyz/geolocation';
const clientIdDiscord = '1189671952479158403';
const revoke_origin = 'https://hypersomnia.xyz';

function decodeCrazyGamesToken(token) {
  const payloadBase64 = token.split('.')[1];
  const payloadDecoded = atob(payloadBase64);
  const payload = JSON.parse(payloadDecoded);
  console.log("Decoded token payload", payload);

  const currentTime = Math.floor(Date.now() / 1000);
  payload.expiresIn = payload.exp - currentTime;
  console.log(currentTime, payload.exp);

  return payload;
}

function downloadAvatar(avatarUrl, callback) {
  fetch(avatarUrl)
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

function openUrl(url) {
  const urlStr = UTF8ToString(url);
  window.open(urlStr, '_blank');
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
  window.history.pushState("object or string", "Title", locStr);
}

function setBrowserLocation_cg(newLocation) {
  if (window.CrazyGames) {
    const locStr = UTF8ToString(newLocation);

    if (locStr === "") {
      window.CrazyGames.SDK.game.hideInviteButton();
    }
    else {
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

const user_config_path = 'user/config.json';

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
        }
        else {
          Module.cg_invite_link = "";
        }
      }

      const available = window.CrazyGames.SDK.user.isUserAccountAvailable;
      console.log("User account system available", available);

      if (available) {
        try {
          const token = await window.CrazyGames.SDK.user.getUserToken();
          console.log("Get token result", token);

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

function pre_run() {
  // Add a run dependency to ensure syncing is done before the application starts
  Module.addRunDependency('idbfs');

  // Create a folder inside our virtual file system
  FS.mkdir('/user');
  // Mount IDBFS on this folder
  FS.mount(IDBFS, {}, '/user');

  // Synchronize from IndexedDB to memory
  FS.syncfs(true, function (err) {
    if (err) console.error('Error loading from IndexedDB', err);
    else console.log('Loaded from IndexedDB');

    // Remove the run dependency after IDBFS is fully loaded
    Module.removeRunDependency('idbfs');
  });
}

function get_cli_args(for_cg) {
  if (for_cg) {
    return "/crazygames";
  }

  var urlPath = window.location.pathname;
  var queryString = window.location.search;

  console.log("urlPath:", urlPath);
  console.log("queryString:", queryString);

  return urlPath + queryString;
}

function sync_idbfs() {
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
      console.error("Filesystem sync failed", err);
      Module.sync_idbfs();
    } else {
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

function revokeDiscord(accessToken) {
  fetch(revoke_origin + '/revoke_discord', {
    method: 'POST',
    headers: {
      'Content-Type': 'application/json'
    },
    body: JSON.stringify({ access_token: accessToken })
  })
    .then(response => response.json())
    .then(data => {
      console.log('Response from server:', data);
    })
    .catch(error => console.error('Error revoking token:', error));
}

function loginDiscord() {
  const site_origin = window.location.origin; // Dynamically get the origin of the current site
  const redirect = site_origin + '/assets/discord_redirect.html';
  const redirectUri = encodeURIComponent(redirect);

  const scope = 'identify';
  const authUrl = `https://discord.com/oauth2/authorize?response_type=token&client_id=${clientIdDiscord}&redirect_uri=${redirectUri}&scope=${scope}`;

  window.open(authUrl, '_blank');
}

function loginCrazyGames() {
  if (window.CrazyGames) {
    window.CrazyGames.SDK.user.showAuthPrompt()
      .then(user => {
        if (user) {
          return window.CrazyGames.SDK.user.getUserToken();
        } else {
          throw new Error('User cancelled login or already signed in');
        }
      })
      .then(token => {
        const payload = decodeCrazyGamesToken(token);

        passAuthDataToCpp('crazygames', payload.userId, payload.username, payload.profilePictureUrl, token, payload.expiresIn);
      })
      .catch(e => {
        console.log("Auth prompt or token retrieval failed:", e);
      });
  }
  else {
    console.log("window.CrazyGames is undefined!");
  }
}

function fetchUserProfile(accessToken, expiresIn) {
  fetch('https://discord.com/api/users/@me', {
    headers: {
      'Authorization': `Bearer ${accessToken}`
    }
  })
    .then(response => response.json())
    .then(profile => {
      const profileName = profile.username;
      const avatarUrl = `https://cdn.discordapp.com/avatars/${profile.id}/${profile.avatar}.png`;

      // Pass data to C++ side
      passAuthDataToCpp('discord', profile.id, profileName, avatarUrl, accessToken, expiresIn);
    })
    .catch(error => {
      console.error('Error fetching Discord user profile:', error);
    });
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

    const channel = new BroadcastChannel('token_bridge');

    channel.addEventListener('message', event => {
      // console.log('Received access token:', event.data.access_token);
      console.log('Expires in:', event.data.expires_in);
      console.log('Token type:', event.data.token_type);

      fetchUserProfile(event.data.access_token, event.data.expires_in);
    });

    Module.channel = channel;

    Module.loginDiscord = loginDiscord;
    Module.revokeDiscord = revokeDiscord;
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
