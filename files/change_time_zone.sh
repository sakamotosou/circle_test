#!/bin/bash
	VALUE=$1
        if [ -n "$VALUE" ] ; then
        echo 'config.time_zone = "${VALUE}"' > config/additional_environment.rb
        bundle exec rails runner "p Time.current" -e test
        bundle exec rake test
        fi
