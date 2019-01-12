# Payload loader

This is a generic payload loader for the Wii U to load arbitrary from the SD Card.  
Currently it's hardcoded to loads a `.elf` file from `sd:/wiiu/payload.elf`.

# Preconditions

This loader expects:  

  - to be able to run at `0x011DD000` (and copied to this place and then executed).
  - to be running inside Mii Maker (for the SD card access),
  - the common `kern_write` (0x35) and `kern_read` (0x34) syscalls installed
  (hooks on `0x0xFFF02234` (write) / `0x0xFFF02214` (read) on FW 5.5.0+)
  - the 0x09 syscall installed which is expected to be a function manipulate
  IBAT0 (`extern void SC_0x09_SETIBAT0(uint32_t upper, uint32_t lower);`)

  Running in any other application with sd access may also work, the IBAT0 setup
  may be to be adjusted though (set back to orignal values at the end)

# Usage
A common usage for this would be to exploit an application, do a kernel exploit
to be able to have kernel read/write, somehow copy the sections of the payload
loader `.elf` file to the expected destination in memory fulfill the mentioned
preconditions.

After that, simply put the `.elf` to be loaded in `sd:/wiiu/payload.elf`

The loaded `.elf` needs to be statically linked somewhere between `0x00800000
and 0x0‭1000000‬`. This whole area is has rwx for both, user and supervisor
(kernel) mode and can be used.  

**This mapping only lasts for this exeuction!** As soon as you leave the running
application (in this case the Mii Maker), the mapping will be reset and you will
loose access to the `0x00800000` region.

# Compiling

In order to be able to compile this, you need to have installed
[devkitPPC](https://devkitpro.org/wiki/Getting_Started) with the following
pacman packages installed.

```
pacman -Syu devkitPPC
```

Make sure the following environment variables are set:
```
DEVKITPRO=/opt/devkitpro
DEVKITPPC=/opt/devkitpro/devkitPPC
```

# Technical details
- This payload loader is supposed to loaded somewhere between
`01000000..01800000` (virtual address), `0x011DD000...0x011E0000` should be free to use.
- The 0x09 syscall is used to set IBAT0 to map `01000000..01800000` (virtual address) to
`32000000..32800000` (physical address) with r/w for user and kernel.
This includes the region where payload loader is, and allows us to register
and execute kernel syscall.
- This setting is meant to match the orignal IBAT0 values (at least in Mii Maker),
but with r/w for the kernel. Resetting is not needed when using the Mii Maker,
but may be needed to be adjusted.
- Afterwards it's possible to register an own syscall (we use 0x36 as it's unused)
to setup IBAT4 and DBAT5 to make `00000000..00800000` (virtual address) to
`30000000..30800000` (physical address) with r/w for user and supervisor.
This allows full user/kernel access to this region, for data and code.
- The mapping is done for all 3 cores.

# Credits

- orboditilt
- dimok789: Most parts (especially sd loading, elf copying) are based on the [homebrew launcher sd loader](https://github.com/dimok789/homebrew_launcher/tree/master/sd_loader).
