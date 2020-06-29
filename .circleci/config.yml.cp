version: 2.1

executors:
  setup:
    docker:
      - image: ruby:2.6-slim-stretch
    environment:
        DEBIAN_FRONTEND: noninteractive
    working_directory: ~/redmine

jobs:
  build:
    executor: setup
    steps:
      - run: apt-get update
      - run: apt-get install -y build-essential zlib1g-dev libssl-dev libreadline-dev libyaml-dev libcurl4-openssl-dev libffi-dev pkg-config
      - run: apt-get install -y apache2 apache2-dev libapr1-dev libaprutil1-dev
      - run: apt-get install -y imagemagick libmagick++-dev
      - run: apt-get install -y cvs mercurial subversion git bzr
      - run: apt-get install -y debconf-utils tzdata wget
      - run: echo 'mysql-server mysql-server/root_password password redmine' | debconf-set-selections       
      - run: echo 'mysql-server mysql-server/root_password_again password redmine' | debconf-set-selections
      - run: wget https://dev.mysql.com/get/mysql-apt-config_0.8.9-1_all.deb
      - run: apt install -y ./mysql-apt-config_0.8.9-1_all.deb
      - run: apt-get update       
      - run: apt-get install -y --allow-unauthenticated mysql-community-server default-libmysqlclient-dev
      - run: /etc/init.d/mysql start
      - run: mysql -u root -predmine -e "CREATE DATABASE redmine CHARACTER SET utf8mb4;"
      - run: mysql -u root -predmine -e "CREATE USER 'redmine'@'localhost' IDENTIFIED BY 'redmine';"
      - run: mysql -u root -predmine -e "GRANT ALL PRIVILEGES ON redmine.* TO 'redmine'@'localhost';"
      - run: mysql -u root -predmine -e "FLUSH PRIVILEGES;"
      - run: apt-get install -y curl unzip      
      - run: wget -q -O - https://dl-ssl.google.com/linux/linux_signing_key.pub | apt-key add -
      - run: echo "deb http://dl.google.com/linux/chrome/deb/ stable main" >> /etc/apt/sources.list.d/google-chrome.list
      - run: rm -rf /var/lib/apt/lists/*
      - run: apt-get update
      - run: apt-get install -y --allow-unauthenticated google-chrome-stable
      - run: wget http://chromedriver.storage.googleapis.com/`curl -sS chromedriver.storage.googleapis.com/LATEST_RELEASE`/chromedriver_linux64.zip       
      - run: unzip chromedriver_linux64.zip
      - run: chmod 755 chromedriver
      - run: mv chromedriver /usr/local/bin/
  install:
    executor: setup
    steps:
      - run: rm -rf redmine
      - run: svn co http://svn.redmine.org/redmine/trunk redmine
      - run: pwd
      - checkout
      - run: pwd
workflows:
  version: 2
  redmine-test:
    jobs:
      - install:
        requires:
          - build
      













