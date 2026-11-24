{ lib, stdenv }:

stdenv.mkDerivation {
  name = "aws-lambda-c";

  src = lib.fileset.toSource {
    root = ./.;
    fileset = lib.fileset.unions [
      ./Makefile
      ./include
      ./src
      ./lambda.pc.in
    ];
  };

  buildFlags = [ "lib" ];
  makeFlags = [ "PREFIX=${placeholder "out"}" ];

  meta = {
    description = "AWS Lambda runtime for C";
    license = lib.licenses.mit;
    platforms = lib.platforms.unix;
  };
}
