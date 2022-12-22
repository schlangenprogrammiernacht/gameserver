use crate::api;

/* You can use the entire Rust standard library (std). */

/**
 * This is your bot's startup function. Here you can set your snake's colors,
 * set up persistent variables, etc.
 */
pub fn init(api: &mut api::Api) -> bool {
    // remove the default color
    api.clear_colors();

    // I'm green!
    api.add_color(40, 255, 0);
    api.add_color(20, 128, 0);
    api.add_color(10, 64, 0);
    api.add_color(20, 128, 0);

    // indicate successful startup. If anything goes wrong,
    // return false and we'll clean you up.
    true
}

/**
 * This function will be called by the framework on every step. Here you decide
 * where to move next!
 *
 * Use the provided Api object to interact with the world. It provides
 * information about the world around you. See the documentation for more
 * details.
 *
 * Your action is specified via the tree return values, which are:
 *
 * - continue (bool): Whether your bot should keep running. Return false here
 *                    for a controlled shutdown (your snake will die and respawn).
 * - angle (f32):     The angle relative to your last heading that you want to
 *                    turn to (in radians, ±pi)
 * - boost (bool):    When set to true, your snake will get a speed boost, but
 *                    this will cost some of the snake’s mass.
 */
pub fn step(api: &mut api::Api) -> (bool, f32, bool) {
    // let's start by moving in a large circle. Please note that all angles are
    // represented in radians, where -π to +π is a full circle.
    let angle: f32 = 0.001;

    // check for other snakes
    let segments = api.get_segments();

    for seg in segments {
        if !seg.is_self && seg.dist < 20.0 {
            // you can send log messages to your browser or any other viewer with the
            // appropriate Viewer Key. It's okay for the call to fail,
            // e.g. when the log memory is full
            let _ = api.log("Oh no, I'm going to die!");
            break;
        }
    }

    // finding food is quite similar

    // Signal that everything is ok. Return false here if anything goes wrong but
    // you want to shut down cleanly.
    (true, angle, false) // continue, angle, boost
}
