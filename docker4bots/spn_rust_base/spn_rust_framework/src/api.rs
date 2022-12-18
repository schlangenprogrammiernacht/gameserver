// vim: noet

extern crate memmap2;

use std::fs::OpenOptions;
use std::mem::{size_of, transmute};
use std::result::Result;

use memmap2::MmapRaw;

pub mod ipc;
use ipc::IpcSharedMemory;

/**
 * Your bot’s interface to the game data.
 *
 * This class provides a safe interface to the shared memory used for fast communication with the
 * gameserver. Use the provided methods to obtain information about your snake’s surroundings or
 * the server configuration, and to access persistent memory.
 *
 * Please note that most methods return direct references to the shared memory structures. These
 * are specified and documented in the ipc module.
 */
pub struct Api<'a> {
    #[allow(dead_code)]
    mmap: MmapRaw,
    ipcdata: &'a mut IpcSharedMemory,
}

impl<'i> Api<'i> {
    /**
     * Construct a new Api instance from the given shared memory file.
     *
     * This function is used internally by the bot framework. Do not worry about it.
     */
    pub fn new(shmfilename: &str) -> Result<Api, String> {
        // open and map the shared memory
        let file = OpenOptions::new()
            .read(true)
            .write(true)
            .open(shmfilename)
            .map_err(|err| format!("Could not open shared memory file {shmfilename}: {err}"))?;

        let mmap = MmapRaw::map_raw(&file)
            .map_err(|err| format!("Could not create shared memory mmap: {err}"))?;

        // check the size of the shared memory. It must be large enough to hold one complete
        // IpcSharedMemory structure.
        if mmap.len() < size_of::<ipc::IpcSharedMemory>() {
            return Err(format!(
                "Shared memory contains only {} bytes where {} bytes are required.",
                mmap.len(),
                size_of::<ipc::IpcSharedMemory>()
            ));
        }

        // map the shared memory to a IpcSharedMemory structure.
        let ipcdata =
            unsafe { &mut *transmute::<*mut u8, *mut ipc::IpcSharedMemory>(mmap.as_mut_ptr()) };

        Ok(Api { mmap, ipcdata })
    }

    /**
     * Get a reference to the server config data.
     *
     * The returned structure contains information about the server
     * configuration and static world information.
     */
    pub fn get_server_config(&self) -> &ipc::IpcServerConfig {
        &self.ipcdata.server_config
    }

    /**
     * Get a pointer to the self information.
     *
     * The returned structure contains information about your snake and
     * parameters of the world.
     */
    pub fn get_self_info(&self) -> &ipc::IpcSelfInfo {
        &self.ipcdata.self_info
    }

    /**
     * Get a reference to the array of food items around your snake’s head.
     *
     * The items are sorted by the distance from your snake’s head, so the first entry is the
     * closest item.
     *
     * Only the valid entries in the shared memory are returned.
     */
    pub fn get_food(&self) -> &[ipc::IpcFoodInfo] {
        &self.ipcdata.food_info[0..self.ipcdata.food_count as usize]
    }

    /**
     * Get a reference to the array of snake segments around your snake’s head.
     *
     * The items are sorted by the distance from your snake’s head, so the first entry is the
     * closest item.
     *
     * Only the valid entries in the shared memory are returned.
     */
    pub fn get_segments(&self) -> &[ipc::IpcSegmentInfo] {
        &self.ipcdata.segment_info[0..self.ipcdata.segment_count as usize]
    }

    /**
     * Get a reference to the array of bot information structures.
     *
     * Only the valid entries in the shared memory are returned.
     */
    pub fn get_bot_info(&self) -> &[ipc::IpcBotInfo] {
        &self.ipcdata.bot_info[0..self.ipcdata.bot_count as usize]
    }

    /**
     * Remove all color entries from the shared memory.
     *
     * This must be called in your [`crate::usercode::init()`] function to remove the default color
     * in case you want to set your own.
     */
    pub fn clear_colors(&mut self) {
        self.ipcdata.color_count = 0;
    }

    /**
     * Add a color to your snake’s color sequence.
     *
     * Call this multiple times from [`crate::usercode::init()`] to set up your snake’s colors. The
     * provided sequence will be repeated along your snake if it has more sequence than colors were
     * specified. You can set up to [`ipc::IPC_COLOR_MAX_COUNT`] colors.
     */
    pub fn add_color(&mut self, r: u8, g: u8, b: u8) {
        self.ipcdata.colors[self.ipcdata.color_count as usize] = ipc::IpcColor { r, g, b };
        self.ipcdata.color_count += 1;
    }

    /**
     * Get a reference to the persistent memory.
     *
     * You can use persistent memory to remember things across multiple lives
     * of your snake. It is saved after your snake dies (even when your code
     * crashes) and restored when it respawns.
     *
     * Note that the size this memory is very limited (given by the
     * [`ipc::IPC_PERSISTENT_MAX_BYTES`] constant). Use it wisely.
     */
    pub fn get_persistent_memory(&mut self) -> &mut [u8] {
        &mut self.ipcdata.persistent_data
    }

    /**
     * Send a log message.
     *
     * These messages will appear on the web interface and in the World update
     * stream when you provide your viewer key to the server.
     *
     * Rate limiting is enforced by the gameserver, so messages are dropped
     * when you send too many of them.
     */
    pub fn log(&mut self, text: &str) -> Result<(), String> {
        // determine length of stored data
        let startpos = self
            .ipcdata
            .log_data
            .iter()
            .position(|&b| b == b'\0')
            .ok_or_else(|| "Log memory is not properly initialized or corrupt".to_string())?;

        if startpos + text.len() + 2 > self.ipcdata.log_data.len() {
            return Err("Rate limit reached (Log memory too full to append this message + newline + nullbyte)".to_owned());
        }

        let mut pos = startpos;
        for byte in text.as_bytes() {
            self.ipcdata.log_data[pos] = *byte;
            pos += 1;
        }

        self.ipcdata.log_data[pos] = b'\n';
        pos += 1;
        self.ipcdata.log_data[pos] = b'\0';

        Ok(())
    }
}
