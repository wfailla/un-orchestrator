#Security Manager

##Overwiew

Security Manager is the component that implements the security features available in the Universal Node orchestrator. Currently, this module exposes the following features:
- User authentication
- User authorization (i.e. permissions)

##Authentication
- The user sends an authentication request to the server through the REST API
- It authentication is successful, the server provides the user a token, which must be attached to each request

##Authorization
- Once the user sends a request, the server checks the authentication token
- If it matches the one stored in the database, it checks the permissions for that operation. The procedure followed by the server can vary depending on the particular operation requested by the user
