// vim: noet

extern crate memmap2;

use std::fs::OpenOptions;
use std::result::Result;
use std::mem::{size_of, transmute};

use memmap2::MmapRaw;

pub mod ipc;
use ipc::IpcSharedMemory;

pub struct Api<'a>
{
	mmap: MmapRaw,
	ipcdata: &'a mut IpcSharedMemory,
}

impl<'i> Api<'i>
{
	pub fn new<'a>(shmfilename: &'a str) -> Result<Api<'a>, String>
	{
		// open and map the shared memory
		let file = OpenOptions::new()
			.read(true)
			.write(true)
			.open(shmfilename).unwrap();

		let mmap = MmapRaw::map_raw(&file).unwrap();

		// check the size of the shared memory. It must be large enough to hold one complete
		// IpcSharedMemory structure.
		if mmap.len() < size_of::<ipc::IpcSharedMemory>() {
			return Err(format!("Shared memory contains only {} bytes where {} bytes are required.",
			       mmap.len(), size_of::<ipc::IpcSharedMemory>()));
		}

		// map the shared memory to a IpcSharedMemory structure.
		let ipcdata = unsafe {
			&mut *transmute::<*mut u8, *mut ipc::IpcSharedMemory>(mmap.as_mut_ptr())
		};

		Ok(Api{
			mmap: mmap,
			ipcdata: ipcdata
		})
	}

	pub fn get_food(&self) -> &[ipc::IpcFoodInfo]
	{
		&self.ipcdata.food_info[0 .. self.ipcdata.food_count as usize]
	}

	pub fn get_segments(&self) -> &[ipc::IpcSegmentInfo]
	{
		&self.ipcdata.segment_info[0 .. self.ipcdata.segment_count as usize]
	}

	pub fn get_bot_info(&self) -> &[ipc::IpcBotInfo]
	{
		&self.ipcdata.bot_info[0 .. self.ipcdata.bot_count as usize]
	}

	pub fn clear_colors(&mut self)
	{
		self.ipcdata.color_count = 0;
	}

	pub fn add_color(&mut self, r: u8, g: u8, b: u8)
	{
		self.ipcdata.colors[self.ipcdata.color_count as usize] = ipc::IpcColor{r, g, b};
		self.ipcdata.color_count += 1;
	}

	pub fn log(&mut self, text: &str) -> bool
	{
		// determine length of stored data
		let startpos;

		match self.ipcdata.log_data.iter().position(|&b| b == b'\0') {
			Some(n) => startpos = n,
			None => return false // log memory is not properly initialized or corrupt
		}

		if startpos + text.len() + 2 > self.ipcdata.log_data.len() {
			// log memory too full to append this message + newline + nullbyte
			return false;
		}

		let mut pos = startpos;
		for byte in text.as_bytes() {
			self.ipcdata.log_data[pos] = *byte;
			pos += 1;
		}

		self.ipcdata.log_data[pos] = b'\n';
		pos += 1;
		self.ipcdata.log_data[pos] = b'\0';
		pos += 1;

		return true;
	}
}
