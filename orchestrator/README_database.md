# Orchestrator Database

UN Orchestrator relies on a local database to support resources management.

## Database structure
![database-structure](https://raw.githubusercontent.com/netgroup-polito/un-orchestrator/permissions/images/Database.JPG)

As shown in the diagram above, the local database is composed by the following tables:
- Users: stores general information about the subscribed users in the node orchestrator
- Login: keeps track of the users that have been authenticated in the system
- Generic resources: lists the available classes of resources
- Current resources permissions: lists the available resources with the related permissions
- User creation permissions: for each user, specify the permission for creating an instance of a generic resource
- Default usage permissions: specifies default permissions to be assigned to the resources belonging to a particular class (i.e. generic resource)
