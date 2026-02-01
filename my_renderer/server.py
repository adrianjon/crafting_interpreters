from livereload import Server

server = Server()

server.watch('demo3/index.html', delay=1)

server.serve(root='.', port=5000)