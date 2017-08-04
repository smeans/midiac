import web
import json

import midiac

urls = (
    '/()(index\.html)?', 'static',
    '/(js|css|images)/(.*)', 'static',
    '/play', 'play',
    '/control', 'control'
    )

app = web.application(urls, globals())

class static:
    def GET(self, media, file):
        if not media and not file:
            file = 'index.html'

        try:
            f = open('static/' + media + '/' + file, 'r')
            return f.read()
        except:
            return None

    def POST(self, media, file):
        return self.GET(media, file)

class play:
    def POST(self):
        ws = web.input(midi_file={})

        midiac.play(ws['midi_file'].file)

        raise web.seeother('/')

class control:
    def POST(self):
        message = json.loads(web.data())

        midiac.control(message)

        return '{"result":"ok"}'

if __name__ == "__main__":
    app.run()
