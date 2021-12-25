// vim: noet
extern crate memmap2;

use std::fs::File;

use memmap2::Mmap;

mod ipc;
use crate::ipc::*;


fn main() {
	println!("Hello World!");

	let file = File::open("test.bin").unwrap();

	let mmap = unsafe { Mmap::map(&file).unwrap() };
}
