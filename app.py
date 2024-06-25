from flask import Flask, request, send_from_directory, session
import subprocess
import os
import uuid
import shutil
import argparse

app = Flask(__name__)
UPLOAD_FOLDER = "uploads"

# 确保上传文件夹存在
os.makedirs(UPLOAD_FOLDER, exist_ok=True)


@app.route("/")
def index():
    return send_from_directory(".", "index.html")


@app.route("/upload", methods=["POST"])
def upload_files():
    # 创建一个唯一的会话目录
    session_id = str(uuid.uuid4())
    session_folder = os.path.join(UPLOAD_FOLDER, session_id)
    os.makedirs(session_folder, exist_ok=True)

    input_file = request.files["inputFile"]
    output_file = request.files["outputFile"]

    input_path = os.path.join(session_folder, "input.txt")
    output_path = os.path.join(session_folder, "output.txt")
    result_path = os.path.join(session_folder, "result.txt")

    input_file.save(input_path)
    output_file.save(output_path)

    # 假设 `cube` 命令可用并且在 PATH 中
    subprocess.run(["cube", input_path, output_path, result_path])

    # 读取并返回 result.txt 的内容
    with open(result_path, "r") as file:
        result_content = file.read()

    # 清理会话目录
    shutil.rmtree(session_folder)

    return result_content


if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument("--host", type=str, default="0.0.0.0")
    parser.add_argument("--port", type=int, default=8123)
    parser.add_argument("--debug", action="store_true")
    parser.add_argument("--wsgi", action="store_true")
    args = parser.parse_args()
    if args.wsgi and args.debug:
        raise ValueError("Cannot use --debug with --wsgi")
    if not args.wsgi:
        app.run(debug=args.debug, host=args.host, port=args.port)
    else:
        from gevent.pywsgi import WSGIServer

        http_server = WSGIServer((args.host, args.port), app)
        http_server.serve_forever()
