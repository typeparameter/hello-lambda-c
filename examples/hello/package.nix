{
  lib,
  stdenv,
  aws-lambda-c,
  nodejs,
  zip,
}:

stdenv.mkDerivation {
  name = "example-hello";

  src = ./.;

  nativeBuildInputs = [ zip ];
  buildInputs = [ aws-lambda-c ];

  buildPhase = ''
    runHook preBuild
    $CC -std=c89 -pedantic -Wall main.c -llambda -o bootstrap
    runHook postBuild
  '';

  installPhase = ''
    runHook preInstall
    install -Dm755 bootstrap $out/bin/bootstrap
    runHook postInstall
  '';

  postFixup = ''
    cp $out/bin/bootstrap bootstrap
    touch -d @$SOURCE_DATE_EPOCH bootstrap
    zip -X $out/lambda.zip bootstrap
  '';

  doInstallCheck = true;
  nativeInstallCheckInputs = [ nodejs ];
  __darwinAllowLocalNetworking = true;

  installCheckPhase = ''
    runHook preInstallCheck
    BOOTSTRAP=$out/bin/bootstrap node --test
    runHook postInstallCheck
  '';

  meta = {
    description = "Example AWS Lambda function built with aws-lambda-c";
    license = lib.licenses.mit;
    platforms = lib.platforms.unix;
    mainProgram = "bootstrap";
  };
}
