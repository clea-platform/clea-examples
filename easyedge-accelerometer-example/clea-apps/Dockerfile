FROM node:14 as builder

WORKDIR /app
ADD . .

RUN cd accelerometer && \
    npm ci && \
    npm run build
RUN cd led && \
    npm ci && \
    npm run build

FROM nginx:1
COPY --from=builder /app/accelerometer/dist/main.js /usr/share/nginx/html/accelerometer/main.js
COPY --from=builder /app/led/dist/main.js /usr/share/nginx/html/led/main.js
ADD nginx.conf /etc/nginx/conf.d/default.conf
