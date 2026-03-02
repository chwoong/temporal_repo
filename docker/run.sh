docker run -it --rm \
  --platform linux/amd64 \
  -v $(pwd):/workspace \
  --name cpp-dev-container \
  chwoong/cpp-dev:13
