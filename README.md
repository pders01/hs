# hs

Minimalist shell prompt in C. "sh" inverted.

## What it does

```
✘ 1 ~/pers/tools/hs on main REBASING +2 !1 ?3 ↑1 ↓2
❯
```

Segments: exit code, directory, git branch, git state, status counts (`+`staged `!`modified `?`untracked `×`conflicted), ahead/behind, stash, prompt char.

## Dependencies

- C11 compiler
- [libgit2](https://libgit2.org/)

## Build

```sh
make
make install        # installs to /usr/local/bin
make install PREFIX=~/.local  # or wherever
```

## Usage

Add to `~/.zshrc`:

```zsh
eval "$(hs init zsh)"
```

Or `~/.bashrc`:

```bash
eval "$(hs init bash)"
```

## Agent mode

Set `HS_AGENT=1` to get JSON instead of a prompt string:

```sh
$ HS_AGENT=1 hs prompt --exit-code=0 --shell=zsh
{"cwd":"/home/user/project","exit_code":0,"git":{"branch":"main","detached":false,"dirty":true,"staged":1,"modified":2,"untracked":0,"conflicted":0,"ahead":1,"behind":0,"stash":0,"state":"none"}}
```

## Configuration

Edit `config.h` and recompile. Colors, symbols, and feature toggles are all `#define`s.

## Nix

As a standalone package (e.g. in an overlay or `pkgs/`):

```nix
{ lib, stdenv, fetchFromGitHub, libgit2, pkg-config }:

stdenv.mkDerivation {
  pname = "hs";
  version = "0.1.0";

  src = fetchFromGitHub {
    owner = "pders01";
    repo = "hs";
    rev = "main";
    hash = ""; # nix will tell you
  };

  nativeBuildInputs = [ pkg-config ];
  buildInputs = [ libgit2 ];
  installFlags = [ "PREFIX=$(out)" ];

  meta = {
    description = "Minimalist shell prompt in C";
    license = lib.licenses.mit;
    platforms = lib.platforms.unix;
  };
}
```

## License

MIT
