docker run -it --rm \
  --network=host \
  --platform linux/amd64 \
  --user $(id -u):$(id -g) \
  -v $(pwd):/workspace \
  -e DISPLAY=$DISPLAY \
  -e XAUTHORITY=/tmp/.Xauthority \
  -v ~/.Xauthority:/tmp/.Xauthority:ro \
  --name cpp-dev-container \
  cpp-dev:13
