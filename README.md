# termtexture
pty rendered to texture

```
GLFW
  |window resize
  |keyboard input
  |mouse input
  V
+---+         +-----+        +---+
|fbo|<-render-|vterm|<=bind=>|pty|/bin/bash
+---+--input->+-----+        +---+
                 A
                bind
                 V
              +--------------+
              |Windows conpty|cmd.exe
              +--------------+pwsh.exe
```
