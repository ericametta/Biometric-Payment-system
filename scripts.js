var db = firebase.database();

// Signs-in Friendly Chat.
function signIn() {
    // Sign in Firebase using popup auth and Google as the identity provider. 
    var provider = new firebase.auth.GoogleAuthProvider(); 
    firebase.auth().signInWithPopup(provider);
}

// Signs-out of the App.
function signOut() {
  // Sign out of Firebase.
  firebase.auth().signOut();
}

// Initiate firebase auth.
function initFirebaseAuth() {
  // Listen to auth state changes.
  firebase.auth().onAuthStateChanged(authStateObserver);
}

// Returns the signed-in user's profile Pic URL. 
function getProfilePicUrl() {
    return firebase.auth().currentUser.photoURL || '/images/profile_placeholder.png'; 
}

// Returns the signed-in user's display name. 
function getUserName() {
  return firebase.auth().currentUser.displayName;
}

function getEmail() {
  return firebase.auth().currentUser.email;
}

function checkAdmin(email) { 
    db.ref('roles/Admin').once('value', function(doc){ 
        var result = doc.val();
        var myArray = Object.values(result);
        var Admin = myArray.includes(email);
        if(Admin){
            console.log("This is what i got: ", result, myArray, Admin);
            loadAdmin();
        }
        else{
            CheckUser(email);
        }
    }).catch(function(error) {
        console.log("Error getting document:", error);
    });
}

function CheckUser(email){ 
    db.ref('roles/user').once('value', function(doc){ 
        var result2 = doc.val();
        var myArray2 = Object.values(result2);
        var User = myArray2.includes(email);
        if(User){
            loadUser(email);
            console.log("Is it User?: ", User);
        }
        else{
            console.log("Unknown User");
        }
    }).catch(function(error) {
        console.log("Error getting document:", error);
    });
}

// Returns true if a user is signed-in. 
function isUserSignedIn() {
  return !!firebase.auth().currentUser;
}

// Triggers when the auth state change for instance when the user signs-in or signs- out.
function authStateObserver(user) {
    if (user) { // User is signed in!
        // Get the signed-in user's profile pic and name.
        var profilePicUrl = getProfilePicUrl();
        var userName = getUserName();
        userEmail = getEmail();

        checkAdmin(userEmail);
        // checkUser(userEmail);
        userPicElement.style.backgroundImage = 'url(' +addSizeToGoogleProfilePic(profilePicUrl) + ')';
        userNameElement.textContent = userName;
        // Show user's profile and sign-out button.
        userNameElement.removeAttribute('hidden');
        userPicElement.removeAttribute('hidden');
        signOutButtonElement.removeAttribute('hidden');
        // Hide sign-in button.
        signInButtonElement.setAttribute('hidden', 'true');
        // We save the Firebase Messaging Device token and enable notifications.
        //saveMessagingDeviceToken();
    }
    else { // User is signed out!
        // Hide user's profile and sign-out button.
        userNameElement.setAttribute('hidden', 'true');
        userPicElement.setAttribute('hidden', 'true');
        signOutButtonElement.setAttribute('hidden', 'true');
        // If an element for that message exists we delete it.
        //screen.setAttribute('hidden', 'true');
        // Show sign-in button.
        signInButtonElement.removeAttribute('hidden');
    }
}
    
// Returns true if user is signed-in. Otherwise false and displays a message. 
function checkSignedInWithMessage() {// Return true if the user is signed in Firebase
    if (isUserSignedIn()) {
        return true;
    }
    // Display a message to the user using a Toast.
    var data = {
        message: 'You must sign-in first',
        timeout: 2000
    };
    signInSnackbarElement.MaterialSnackbar.showSnackbar(data);
    return false;
}

// Adds a size to Google Profile pics URLs. 
function addSizeToGoogleProfilePic(url) {
    if (url.indexOf('googleusercontent.com') !== -1 && url.indexOf('?') === -1) { 
        return url + '?sz=150';
    }
    return url; 
}

// Template for messages.
var MESSAGE_TEMPLATE =
  `<table class="message-container">
    <th class="acct_no"></th>
    <th class="name"></th>
    <th class="Transxn"></th>
    <th class="Transxn_id"></th>
    <th class="balance"></th>
    </table>`;

function createAndInsertMessage(id, date) {
    const container = document.createElement('div'); 
    container.innerHTML = MESSAGE_TEMPLATE;
    const div = container.firstChild;
    div.setAttribute('id', id);
   
    // If date is null, assume we've gotten a brand new message.
    // https://stackoverflow.com/a/47781432/4816918
    date = Date.now();
    div.setAttribute('date', date);
    
    // figure out where to insert new message
    const existingMessages = messageListElement.children;
    if (existingMessages.length === 0) {
        messageListElement.appendChild(div);
    } 
    else {
        let messageListNode = existingMessages[0];
        while (messageListNode) {
            const messageListNodeTime = messageListNode.getAttribute('date');
            if (!messageListNodeTime) {
                throw new Error(
                `Child ${messageListNode.id} has no 'date' attribute`
                );
            }
            if (messageListNodeTime > date) {
                break;
            }
            messageListNode = messageListNode.nextSibling;
        }
        messageListElement.insertBefore(div, messageListNode);
    }
    return div; 
}

// Displays a Message in the UI.
function displayMessage(id, date, name, email, balance, acct_no, Transxn) {
    var div = document.getElementById(id) || createAndInsertMessage(id, date);
        div.querySelector('.Transxn_id').textContent = id; 
        div.querySelector('.name').textContent = name; 
        div.querySelector('.acct_no').textContent=acct_no; 
        div.querySelector('.Transxn').textContent= Transxn; 
        div.querySelector('.balance').textContent= balance;

    // Show the card fading-in and scroll to view the new message. 
    setTimeout(function() {div.classList.add('visible')}, 1); 
    messageListElement.scrollTop = messageListElement.scrollHeight; 
    //messageInputElement.focus();
}


function loadAdmin() { 
    db.ref('Transactions').on('value', function(snapshot) {
        snapshot.forEach(function(change) {
            var id = (change.val() && change.key);
            // var date = (change.val() && change.val().date); 
            // var time = date.toString();
            var message = change.val();
            displayMessage(
                id, 
                message.date, 
                message.name,
                message.email, 
                message.balance, 
                message.acct_no,
                message.Transxn
            );
            console.log('key here>>', id, 'data>>', message.Transxn);
        }); 
    });
}

function loadUser(email) {
    db.ref('Transactions').orderByChild("email").equalTo(email).on('value', function(snapshot) {
        snapshot.forEach(function(change) {
            var id = (change.val() && change.key);
            var date = (change.val() && change.val().date);
            var time = date.toString();
            var message = change.val();
            displayMessage(id, 
                time, 
                message.name, 
                message.email, 
                message.balance, 
                message.acct_no, 
                message.Transxn
            );
            console.log('key here>>', id, time, 'data>>', message.Transxn);
        }); 
    });
}

// Shortcuts to DOM Elements.
var screen = document.getElementById('messages-card');
var messageListElement = document.getElementById('messages');
var messageFormElement = document.getElementById('message-form');
var messageInputElement = document.getElementById('message');
var mediaCaptureElement = document.getElementById('mediaCapture');
var userPicElement = document.getElementById('user-pic');
var userNameElement = document.getElementById('user-name');
var signInButtonElement = document.getElementById('sign-in');
var signOutButtonElement = document.getElementById('sign-out');
var signInSnackbarElement = document.getElementById('must-signin-snackbar');

signOutButtonElement.addEventListener('click', signOut);
signInButtonElement.addEventListener('click', signIn);

// initialize Firebase Authentication
initFirebaseAuth();