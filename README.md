# Flareboard Archiver

**[Flareboard](https://flareboard.ru/) web archiver in C using libcurl**

It saves:

- [x] Thread HTML page
- [x] CSS styles from thread
- [x] JS scripts from thread
- [x] Attached images and videos from thread
- [x] Comments in thread (without running JS code)
- [x] Correct message timestamps
- [x] User profile photos

This tool also uses `pthread` to enable multithreading page downloading.

## Build

- GNU/Linux, *BSD and other *nix
- Windows
  with [CMake](https://community.chocolatey.org/packages/cmake), [Ninja](https://community.chocolatey.org/packages/ninja)
  and [MinGW](https://community.chocolatey.org/packages/mingw)

On GNU/Linux you need to install these libraries first:

- libcurl (Debian: `libcurl4`, Arch: `curl`)

```bash
git clone https://github.com/meequrox/flb-archiver.git

cd flb-archiver

cmake --preset=release

cmake --build --preset=release
```

The compiled files will be located in the `build/release/bin` Directory.

## Usage

```bash
# Print usage hint
./flb-archiver

# Archive threads with ID from 1 to 100
./flb-archiver 1 100

# Archive all threads
# UB is the ID of the last post in the feed
./flb-archiver 1 <UB>
```

## Example

```bash
$ time ./flb-archiver 1 3288
      ...
INFO> main(): ID range [1; 3288]
INFO> main(): Using interval 150 ms
      ...
INFO> flb_download_threads(): Workers: 16; FLB threads: 3288; Threads per worker (avg): 205
      ...

real	2m29,082s
user	0m15,327s
sys     0m9,978s
```
