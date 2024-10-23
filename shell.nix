{pkgs}: {
  default = pkgs.mkShell {
    nativeBuildInputs = with pkgs; [
      cloc
      ns-2 # Network Simulator 2 from personal nur-packages
    ];
  };
}
