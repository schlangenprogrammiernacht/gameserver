// vim: noet

extern crate memmap2;

use std::fs::OpenOptions;
use std::result::Result;
use std::mem::{size_of, transmute};

use memmap2::MmapRaw;

mod ipc;
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
		let file = OpenOptions::new()
			.read(true)
			.write(true)
			.open(shmfilename).unwrap();

		let mmap = MmapRaw::map_raw(&file).unwrap();

		if mmap.len() < size_of::<ipc::IpcSharedMemory>() {
			return Err(format!("Shared memory contains only {} bytes where {} bytes are required.",
			       mmap.len(), size_of::<ipc::IpcSharedMemory>()));
		}

		let ipcdata = unsafe {
			&mut *transmute::<*mut u8, *mut ipc::IpcSharedMemory>(mmap.as_mut_ptr())
		};

		println!("Food Count: {}", ipcdata.food_count);

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
}
