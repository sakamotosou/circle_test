version: 2.1

executors:
  setup:
    docker:
      - image: ruby:2.6-slim-stretch
    environment:
        DEBIAN_FRONTEND: noninteractive
        RUBYOPT: -EUTF-8
commands:
  build:
    description: "build"
    steps:
      - checkout
      - run: ls
      - run: find ./ -name database.yml
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
    description: "install"
    steps:
      - checkout
      - run: ls
      - run: find ./ -name database.yml
      - run: rm -rf redmine
      - run: git clone https://github.com/redmica/redmica.git -b stable-1.1 redmine
      - run: cp ./files/database.yml ./redmine/config/database.yml
      - run:
          working_directory: redmine
          command: |
            ls
            cat ./config/database.yml
            cat ./Gemfile
            bundle install
            cp ../files/change_time_zone.sh change_time_zone.sh
            cp ../files/skip_test.rb skip_test.rb
            ls
            rm -rf plugins/redmine_issue_templates
            git clone https://github.com/akiko-pusu/redmine_issue_templates plugins/redmine_issue_templates
            ls
            ls plugins/redmine_issue_templates/
            cp plugins/redmine_issue_templates/Gemfile.local plugins/redmine_issue_templates/Gemfile
            rm -rf plugins/view_customize
            git clone https://github.com/onozaty/redmine-view-customize plugins/view_customize
            rm -rf plugins/redmine_vividtone_my_page_blocks
            git clone https://github.com/vividtone/redmine_vividtone_my_page_blocks plugins/redmine_vividtone_my_page_blocks
            bundle install
            bundle exec rails db:environment:set RAILS_ENV=test
            bundle exec rake generate_secret_token
            bundle exec rake db:migrate RAILS_ENV=test
            bundle exec rake test:scm:setup:all
            bundle exec rake test:scm:update
            bundle exec rake redmine:plugins:migrate NAME=redmine_issue_templates RAILS_ENV=test
            bundle exec rake redmine:plugins:migrate NAME=view_customize RAILS_ENV=test
            bundle exec rake redmine:plugins:migrate NAME=redmine_vividtone_my_page_blocks RAILS_ENV=test
  test-utc:
    description: "test-utc"
    steps:
      - checkout
      - run:
          working_directory: redmine
          command: |
            ls
            touch build_result.txt
            sh change_time_zone.sh UTC > build_result.txt
            cat build_result.txt
      - run:
          command: |
            echo "fail test-utc"
            cat build_result.txt
          when: on_fail
  test-tokyo:
    description: "test-tokyo"
    steps:
      - checkout
      - run:
          working_directory: redmine
          command: |
            ls
            ruby skip_test.rb TOKYO_SKIP_TESTS
            sh change_time_zone.sh Tokyo > build_result.txt
            cat build_result.txt
      - run:
          command: |
            echo "fail test-tokyo"
            cat build_result.txt
          when: on_fail
  test-american_samoa:
    description: "test-american_samoa"
    steps:
      - checkout
      - run:
          working_directory: redmine
          command: |
            ls
            ruby skip_test.rb SAMOA_SKIP_TESTS
            sh change_time_zone.sh "American Samoa" > build_result.txt
            cat build_result.txt
      - run:
          command: |
            echo "fail test-american_samoa"
            cat build_result.txt
          when: on_fail
  test-redmine_issue_templates:
    description: "test-redmine_issue_templates"
    steps:
      - checkout
      - run:
          working_directory: redmine
          command: |
            bundle exec rake redmine:plugins:test NAME=redmine_issue_templates RAILS_ENV=test > build_result.txt
            cat build_result.txt
            sed -i "/Options.new/s/$/(args: [\'--headless\', \'--no-sandbox\', \'--disable-gpu\', \'--window-size=1280,800\'])/g" plugins/redmine_issue_templates/spec/rails_helper.rb
      - run:
          command: |
            echo "fail test-redmine_issue_templates"
            cat build_result.txt
          when: on_fail
jobs:
  redmine-test:
    executor:
      name: setup
    steps:
      - build
      - install
      - test-tokyo
      - test-american_samoa
      - test-redmine_issue_templates
workflows:
  normal:
    jobs:
      - redmine-test
      














