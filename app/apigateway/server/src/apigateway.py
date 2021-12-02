# Copyright (C) 2020 Georgia Tech Center for Experimental Research in Computer
# Systems

import random

import flask
import flask_httpauth
import spdlog as spd
import yaml

from buzzblog.account_client import Client as AccountClient
from buzzblog.follow_client import Client as FollowClient
from buzzblog.like_client import Client as LikeClient
from buzzblog.post_client import Client as PostClient
from buzzblog.gen.ttypes import *


class ThriftClientFactory:
    def __init__(self):
        backend_filename = "/etc/opt/BuzzBlogApp/backend.yml"
        with open(backend_filename, encoding="utf-8") as backend_file:
            backend = yaml.safe_load(backend_file)
            self._account_servers = backend["account"]["service"]
            self._follow_servers = backend["follow"]["service"]
            self._like_servers = backend["like"]["service"]
            self._post_servers = backend["post"]["service"]

    def get_account_client(self):
        server = random.choice(self._account_servers)
        return AccountClient(server.split(':')[0], int(server.split(':')[1]))

    def get_follow_client(self):
        server = random.choice(self._follow_servers)
        return FollowClient(server.split(':')[0], int(server.split(':')[1]))

    def get_like_client(self):
        server = random.choice(self._like_servers)
        return LikeClient(server.split(':')[0], int(server.split(':')[1]))

    def get_post_client(self):
        server = random.choice(self._post_servers)
        return PostClient(server.split(':')[0], int(server.split(':')[1]))


def setup_app():
    application = flask.Flask(__name__)
    application.url_map.strict_slashes = False
    return application

def setup_logger():
    logger = spd.FileLogger("logger", "/tmp/calls.log")
    logger.set_pattern("[%H:%M:%S.%F] pid=%P tid=%t %v")
    return logger


app = setup_app()
auth = flask_httpauth.HTTPBasicAuth()
thrift_client_factory = ThriftClientFactory()
logger = setup_logger()


@auth.verify_password
def verify_password(username, password):
    request_metadata = TRequestMetadata(id=flask.request.args["request_id"])
    with thrift_client_factory.get_account_client() as account_client:
        try:
            account = account_client.authenticate_user(
                request_metadata=request_metadata, username=username,
                password=password)
        except ValueError:
            account = None
    return account


@app.route("/account", methods=["POST"])
def create_account():
    request_metadata = TRequestMetadata(id=flask.request.args["request_id"])
    params = flask.request.get_json()
    try:
        username = params["username"]
        password = params["password"]
        first_name = params["first_name"]
        last_name = params["last_name"]
    except KeyError:
        return ({}, 400)
    with thrift_client_factory.get_account_client() as account_client:
        try:
            account = account_client.create_account(request_metadata=request_metadata,
                username=username, password=password, first_name=first_name,
                last_name=last_name)
        except TAccountInvalidAttributesException:
            return ({}, 400)
        except TAccountUsernameAlreadyExistsException:
            return ({}, 400)
    return {
      "object": "account",
      "mode": "standard",
      "id": account.id,
      "created_at": account.created_at,
      "active": account.active,
      "username": account.username,
      "first_name": account.first_name,
      "last_name": account.last_name
    }


@app.route("/account/<int:account_id>", methods=["GET"])
@auth.login_required
def retrieve_account(account_id):
    request_metadata = TRequestMetadata(id=flask.request.args["request_id"],
        requester_id=auth.current_user().id)
    with thrift_client_factory.get_account_client() as account_client:
        try:
            account = account_client.retrieve_expanded_account(
                request_metadata=request_metadata, account_id=account_id)
        except TAccountNotFoundException:
            return ({}, 404)
    return {
      "object": "account",
      "mode": "expanded",
      "id": account.id,
      "created_at": account.created_at,
      "active": account.active,
      "username": account.username,
      "first_name": account.first_name,
      "last_name": account.last_name,
      "follows_you": account.follows_you,
      "followed_by_you": account.followed_by_you,
      "n_followers": account.n_followers,
      "n_following": account.n_following,
      "n_posts": account.n_posts,
      "n_likes": account.n_likes
    }


@app.route("/account/<int:account_id>", methods=["PUT"])
@auth.login_required
def update_account(account_id):
    request_metadata = TRequestMetadata(id=flask.request.args["request_id"],
        requester_id=auth.current_user().id)
    params = flask.request.get_json()
    try:
        password = params["password"]
        first_name = params["first_name"]
        last_name = params["last_name"]
    except KeyError:
        return ({}, 400)
    with thrift_client_factory.get_account_client() as account_client:
        try:
            account = account_client.update_account(
                request_metadata=request_metadata, account_id=account_id,
                password=password, first_name=first_name, last_name=last_name)
        except TAccountInvalidAttributesException:
            return ({}, 400)
        except TAccountNotAuthorizedException:
            return ({}, 403)
        except TAccountNotFoundException:
            return ({}, 404)
    return {
      "object": "account",
      "mode": "standard",
      "id": account.id,
      "created_at": account.created_at,
      "active": account.active,
      "username": account.username,
      "first_name": account.first_name,
      "last_name": account.last_name
    }


@app.route("/account/<int:account_id>", methods=["DELETE"])
@auth.login_required
def delete_account(account_id):
    request_metadata = TRequestMetadata(id=flask.request.args["request_id"],
        requester_id=auth.current_user().id)
    with thrift_client_factory.get_account_client() as account_client:
        try:
            account_client.delete_account(request_metadata=request_metadata,
                account_id=account_id)
        except TAccountNotAuthorizedException:
            return ({}, 403)
        except TAccountNotFoundException:
            return ({}, 404)
    return {}


@app.route("/follow", methods=["POST"])
@auth.login_required
def follow_account():
    request_metadata = TRequestMetadata(id=flask.request.args["request_id"],
        requester_id=auth.current_user().id)
    params = flask.request.get_json()
    try:
        account_id = params["account_id"]
    except KeyError:
        return ({}, 400)
    with thrift_client_factory.get_follow_client() as follow_client:
        try:
            follow = follow_client.follow_account(request_metadata=request_metadata,
                account_id=account_id)
        except TFollowAlreadyExistsException:
            return ({}, 400)
    return {
      "object": "follow",
      "mode": "standard",
      "id": follow.id,
      "created_at": follow.created_at,
      "follower_id": follow.follower_id,
      "followee_id": follow.followee_id
    }


@app.route("/follow/<int:follow_id>", methods=["GET"])
@auth.login_required
def retrieve_follow(follow_id):
    request_metadata = TRequestMetadata(id=flask.request.args["request_id"],
        requester_id=auth.current_user().id)
    with thrift_client_factory.get_follow_client() as follow_client:
        try:
            follow = follow_client.retrieve_expanded_follow(
                request_metadata=request_metadata, follow_id=follow_id)
        except TFollowNotFoundException:
            return ({}, 404)
    return {
      "object": "follow",
      "mode": "expanded",
      "id": follow.id,
      "created_at": follow.created_at,
      "follower_id": follow.follower_id,
      "followee_id": follow.followee_id,
      "follower": {
        "object": "account",
        "mode": "standard",
        "id": follow.follower.id,
        "created_at": follow.follower.created_at,
        "active": follow.follower.active,
        "username": follow.follower.username,
        "first_name": follow.follower.first_name,
        "last_name": follow.follower.last_name
      },
      "followee": {
        "object": "account",
        "mode": "standard",
        "id": follow.followee.id,
        "created_at": follow.followee.created_at,
        "active": follow.followee.active,
        "username": follow.followee.username,
        "first_name": follow.followee.first_name,
        "last_name": follow.followee.last_name
      }
    }


@app.route("/follow/<int:follow_id>", methods=["DELETE"])
@auth.login_required
def delete_follow(follow_id):
    request_metadata = TRequestMetadata(id=flask.request.args["request_id"],
        requester_id=auth.current_user().id)
    with thrift_client_factory.get_follow_client() as follow_client:
        try:
            follow_client.delete_follow(request_metadata=request_metadata,
                follow_id=follow_id)
        except TFollowNotAuthorizedException:
            return ({}, 403)
        except TFollowNotFoundException:
            return ({}, 404)
    return {}


@app.route("/follow", methods=["GET"])
@auth.login_required
def list_follows():
    request_metadata = TRequestMetadata(id=flask.request.args["request_id"],
        requester_id=auth.current_user().id)
    params = flask.request.get_json()
    try:
        limit = params["limit"]
        offset = params["offset"]
    except KeyError:
        return ({}, 400)
    follower_id = int(flask.request.args["follower_id"]) \
        if "follower_id" in flask.request.args else None
    followee_id = int(flask.request.args["followee_id"]) \
        if "followee_id" in flask.request.args else None
    query = TFollowQuery(follower_id=follower_id, followee_id=followee_id)
    with thrift_client_factory.get_follow_client() as follow_client:
        try:
            follows = follow_client.list_follows(request_metadata=request_metadata,
                query=query, limit=limit, offset=offset)
        except TAccountNotFoundException:
            return ({}, 400)
    return flask.jsonify([{
      "object": "follow",
      "mode": "expanded",
      "id": follow.id,
      "created_at": follow.created_at,
      "follower_id": follow.follower_id,
      "followee_id": follow.followee_id,
      "follower": {
        "object": "account",
        "mode": "standard",
        "id": follow.follower.id,
        "created_at": follow.follower.created_at,
        "active": follow.follower.active,
        "username": follow.follower.username,
        "first_name": follow.follower.first_name,
        "last_name": follow.follower.last_name
      },
      "followee": {
        "object": "account",
        "mode": "standard",
        "id": follow.followee.id,
        "created_at": follow.followee.created_at,
        "active": follow.followee.active,
        "username": follow.followee.username,
        "first_name": follow.followee.first_name,
        "last_name": follow.followee.last_name
      }
    } for follow in follows])


@app.route("/post", methods=["POST"])
@auth.login_required
def create_post():
    request_metadata = TRequestMetadata(id=flask.request.args["request_id"],
        requester_id=auth.current_user().id)
    params = flask.request.get_json()
    try:
        text = params["text"]
    except KeyError:
        return ({}, 400)
    with thrift_client_factory.get_post_client() as post_client:
        try:
            post = post_client.create_post(request_metadata=request_metadata,
                text=text)
        except TPostInvalidAttributesException:
            return ({}, 400)
    return {
      "object": "post",
      "mode": "standard",
      "id": post.id,
      "created_at": post.created_at,
      "active": post.active,
      "text": post.text,
      "author_id": post.author_id
    }


@app.route("/post/<int:post_id>", methods=["GET"])
@auth.login_required
def retrieve_post(post_id):
    request_metadata = TRequestMetadata(id=flask.request.args["request_id"],
        requester_id=auth.current_user().id)
    with thrift_client_factory.get_post_client() as post_client:
        try:
            post = post_client.retrieve_expanded_post(
                request_metadata=request_metadata, post_id=post_id)
        except TPostNotFoundException:
            return ({}, 404)
    return {
      "object": "post",
      "mode": "expanded",
      "id": post.id,
      "created_at": post.created_at,
      "active": post.active,
      "text": post.text,
      "author_id": post.author_id,
      "author": {
        "object": "account",
        "mode": "standard",
        "id": post.author.id,
        "created_at": post.author.created_at,
        "active": post.author.active,
        "username": post.author.username,
        "first_name": post.author.first_name,
        "last_name": post.author.last_name
      },
      "n_likes": post.n_likes
    }


@app.route("/post/<int:post_id>", methods=["DELETE"])
@auth.login_required
def delete_post(post_id):
    request_metadata = TRequestMetadata(id=flask.request.args["request_id"],
        requester_id=auth.current_user().id)
    with thrift_client_factory.get_post_client() as post_client:
        try:
            post_client.delete_post(request_metadata=request_metadata,
                post_id=post_id)
        except TPostNotAuthorizedException:
            return ({}, 403)
        except TPostNotFoundException:
            return ({}, 404)
    return {}


@app.route("/post", methods=["GET"])
@auth.login_required
def list_posts():
    request_metadata = TRequestMetadata(id=flask.request.args["request_id"],
        requester_id=auth.current_user().id)
    params = flask.request.get_json()
    try:
        limit = params["limit"]
        offset = params["offset"]
    except KeyError:
        return ({}, 400)
    author_id = int(flask.request.args["author_id"]) \
        if "author_id" in flask.request.args else None
    query = TPostQuery(author_id=author_id)
    with thrift_client_factory.get_post_client() as post_client:
        try:
            posts = post_client.list_posts(request_metadata=request_metadata,
                query=query, limit=limit, offset=offset)
        except TAccountNotFoundException:
            return ({}, 400)
    return flask.jsonify([{
      "object": "post",
      "mode": "expanded",
      "id": post.id,
      "created_at": post.created_at,
      "active": post.active,
      "text": post.text,
      "author_id": post.author_id,
      "author": {
        "object": "account",
        "mode": "standard",
        "id": post.author.id,
        "created_at": post.author.created_at,
        "active": post.author.active,
        "username": post.author.username,
        "first_name": post.author.first_name,
        "last_name": post.author.last_name
      },
      "n_likes": post.n_likes
    } for post in posts])


@app.route("/like", methods=["POST"])
@auth.login_required
def like_post():
    request_metadata = TRequestMetadata(id=flask.request.args["request_id"],
        requester_id=auth.current_user().id)
    params = flask.request.get_json()
    try:
        post_id = params["post_id"]
    except KeyError:
        return ({}, 400)
    with thrift_client_factory.get_like_client() as like_client:
        try:
            like = like_client.like_post(request_metadata=request_metadata,
                post_id=post_id)
        except TLikeAlreadyExistsException:
            return ({}, 400)
    return {
      "object": "like",
      "mode": "standard",
      "id": like.id,
      "created_at": like.created_at,
      "account_id": like.account_id,
      "post_id": like.post_id
    }


@app.route("/like/<int:like_id>", methods=["GET"])
@auth.login_required
def retrieve_like(like_id):
    request_metadata = TRequestMetadata(id=flask.request.args["request_id"],
        request_metadata=request_metadata)
    with thrift_client_factory.get_like_client() as like_client:
        try:
            like = like_client.retrieve_like(request_metadata=request_metadata,
                like_id=like_id)
        except TLikeNotFoundException:
            return ({}, 404)
    return {
      "object": "like",
      "mode": "expanded",
      "id": like.id,
      "created_at": like.created_at,
      "account_id": like.account_id,
      "post_id": like.post_id,
      "account": {
        "object": "account",
        "mode": "standard",
        "id": like.account.id,
        "created_at": like.account.created_at,
        "active": like.account.active,
        "username": like.account.username,
        "first_name": like.account.first_name,
        "last_name": like.account.last_name
      },
      "post": {
        "object": "post",
        "mode": "expanded",
        "id": like.post.id,
        "created_at": like.post.created_at,
        "active": like.post.active,
        "text": like.post.text,
        "author_id": like.post.author_id,
        "author": {
          "object": "account",
          "mode": "standard",
          "id": like.post.author.id,
          "created_at": like.post.author.created_at,
          "active": like.post.author.active,
          "username": like.post.author.username,
          "first_name": like.post.author.first_name,
          "last_name": like.post.author.last_name
        },
        "n_likes": like.post.n_likes
      }
    }


@app.route("/like/<int:like_id>", methods=["DELETE"])
@auth.login_required
def delete_like(like_id):
    request_metadata = TRequestMetadata(id=flask.request.args["request_id"],
        requester_id=auth.current_user().id)
    with thrift_client_factory.get_like_client() as like_client:
        try:
            like_client.delete_like(request_metadata=request_metadata,
                like_id=like_id)
        except TLikeNotAuthorizedException:
            return ({}, 403)
        except TLikeNotFoundException:
            return ({}, 404)
    return {}


@app.route("/like", methods=["GET"])
@auth.login_required
def list_likes():
    request_metadata = TRequestMetadata(id=flask.request.args["request_id"],
        requester_id=auth.current_user().id)
    params = flask.request.get_json()
    try:
        limit = params["limit"]
        offset = params["offset"]
    except KeyError:
        return ({}, 400)
    account_id = int(flask.request.args["account_id"]) \
        if "account_id" in flask.request.args else None
    post_id = int(flask.request.args["post_id"]) \
        if "post_id" in flask.request.args else None
    query = TLikeQuery(account_id=account_id, post_id=post_id)
    with thrift_client_factory.get_like_client() as like_client:
        try:
            likes = like_client.list_likes(request_metadata=request_metadata,
                query=query, limit=limit, offset=offset)
        except TAccountNotFoundException:
            return ({}, 400)
        except TPostNotFoundException:
            return ({}, 400)
    return flask.jsonify([{
      "object": "like",
      "mode": "expanded",
      "id": like.id,
      "created_at": like.created_at,
      "account_id": like.account_id,
      "post_id": like.post_id,
      "account": {
        "object": "account",
        "mode": "standard",
        "id": like.account.id,
        "created_at": like.account.created_at,
        "active": like.account.active,
        "username": like.account.username,
        "first_name": like.account.first_name,
        "last_name": like.account.last_name
      },
      "post": {
        "object": "post",
        "mode": "expanded",
        "id": like.post.id,
        "created_at": like.post.created_at,
        "active": like.post.active,
        "text": like.post.text,
        "author_id": like.post.author_id,
        "author": {
          "object": "account",
          "mode": "standard",
          "id": like.post.author.id,
          "created_at": like.post.author.created_at,
          "active": like.post.author.active,
          "username": like.post.author.username,
          "first_name": like.post.author.first_name,
          "last_name": like.post.author.last_name
        },
        "n_likes": like.post.n_likes
      }
    } for like in likes])
