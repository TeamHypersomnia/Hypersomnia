# Sourced by every script that starts the game for a test - the one place for what every test game gets:
# silent (OpenAL's null backend, and master volume 0), and nothing opened on the user's screen
# ($VISUAL is a no-op, a failure log is never opened - read it from <appdata>/logs/ yourself).
# Use: timeout 60 $BIN --connect ... --appdata-dir $T/c $GAME_TEST_FLAGS > $T/c.out 2>&1
export ALSOFT_DRIVERS=null VISUAL=true
GAME_TEST_FLAGS="--no-update-on-launch --mute --no-open-logs"
