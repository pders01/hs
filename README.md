# hs

Minimalist shell prompt in C. "sh" inverted.

## What it does

```
~/Projects/hs :: main +2!1?3 ↑1↓2 ./ 
```

```

Single line: exit code, directory, git context, prompt. Context appears only when relevant.

`+`staged `!`modified `?`untracked `x`conflicted `↑`ahead `↓`behind `~`stash `&`jobs

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
{"cwd":"/home/user/project","exit_code":0,"duration_ms":0,"jobs":0,"ssh":false,"git":{"branch":"main","detached":false,"dirty":true,"staged":1,"modified":2,"untracked":0,"conflicted":0,"ahead":1,"behind":0,"stash":0,"state":"none"}}
```

## Configuration

Edit `config.h` and recompile. Colors, symbols, thresholds, and feature toggles are all `#define`s.

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
