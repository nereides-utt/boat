# test_api.py
from flask import Flask, request, jsonify

app = Flask(__name__)

@app.route('/test', methods=['POST'])
def test_endpoint():
    data = request.json
    print(f"Data received: {data}")
    return jsonify({'status': 'success', 'received_data': data}), 200

if __name__ == '__main__':
    app.run(debug=True)


#pour test: python test_api.py