{ pkgs ? import <nixpkgs> {} }:

let
  python = pkgs.python313;
  pythonEnv = python.withPackages (ps: with ps; [
    fastapi
    uvicorn
    sqlmodel
    aiosqlite
    python-multipart
    bcrypt
    python-jose
    cryptography
    # uvicorn[standard] extras
    websockets
    httptools
    uvloop
    watchfiles
  ]);
in
pkgs.mkShell {
  packages = [ pythonEnv ];

  shellHook = ''
    echo "FACEIT dev env ready — run 'make run' to start"
  '';
}
