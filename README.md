# flb-archiver
**[Flareboard](https://flareboard.ru/) web archiver in C using libcurl**

It saves:
- Thread page (w/o deleted posts)
- CSS styles and JS scripts
- Pinned images/videos
- User profile photos

Save not implemented:
- Thread comments (loaded dynamically with JS)

## Installation

You need to install this libraries first:
- libxml2 (Debian: `libxml2-dev`, Arch: `libxml2`)
- libcurl (Debian: `libcurl4`, Arch: `curl`)

```bash
git clone https://github.com/meequrox/flb-archiver.git

cd flb-archiver/build

cmake ..

make
```

## Usage

```bash
# Print usage hint
./flb-archiver

# Archive threads with id 1-100
./flb-archiver 1 100

# Archive all threads
# UB is the id of the last post in the feed
./flb-archiver 1 UB
```
