docker run -it --rm \
  --platform linux/amd64 \
  -v $(pwd):/workspace \
  --name cpp-dev-container \
  cpp-dev:13
