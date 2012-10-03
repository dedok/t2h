#!/bin/bash

pwd=$(pwd)
full_model_path=$pwd/bin/full_model
full_model_args="--with-config=$pwd/t2h_envt/config.json --torrents-dir=$pwd/t2h_envt/torrents/"
build_dir="$pwd/static_build"
build_target_name="full_model"

function exit_failure() {
	echo "Error detected: $@." && exit 1
	}

function build_and_run_t2h() {
	local make_command="make -C $build_dir $build_target_name"
	local full_model_command="$full_model_path $full_model_args"
	(echo Command : $make_command && eval $make_command) || \
		exit_failure "$make_command failed"
	(echo Command : $full_model_command && eval $full_model_command) || \
		exit_failure "$full_model_command failed"
	}

[ ! -d "$pwd/t2h_envt" ] && exit_failure "$pwd/t2h_envt not exists or ill formed"
build_and_run_t2h

