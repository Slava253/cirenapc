let allNotes = [];


// загрузка заметок

async function loadNotes(){

    let response = await fetch("/api/notes");

    let data = await response.json();

    allNotes = data.notes || [];


    let search =
        document.getElementById("search").value
        .toLowerCase();



    let container =
        document.getElementById("notes");


    container.innerHTML = "";



    allNotes
    .filter(note => {

        return (
            note.title.toLowerCase().includes(search) ||
            note.text.toLowerCase().includes(search)
        );

    })
    .forEach(note => {


        let div =
        document.createElement("div");


        div.className =
        "note " +
        (note.favorite ? "favorite" : "");



        div.innerHTML = `


        <h3>
        ${note.favorite ? "⭐ " : ""}
        ${note.title}
        </h3>


        <p>${note.text}</p>


        <small>
        ID: ${note.id}
        </small>


        <div class="note-buttons">


        <button onclick="favoriteNote(${note.id})">
        ⭐
        </button>


        <button onclick="editNote(${note.id})">
        ✏️
        </button>


        <button onclick="deleteNote(${note.id})">
        🗑
        </button>


        </div>


        `;


        container.appendChild(div);


    });


}




// добавить заметку

async function addNote(){


    let title =
    document.getElementById("title").value;


    let text =
    document.getElementById("text").value;



    if(title.trim()==="" || text.trim()===""){

        alert("Заполните название и текст");

        return;
    }



    await fetch("/api/add",{


        method:"POST",

        headers:{
            "Content-Type":"application/json"
        },


        body:JSON.stringify({

            title:title,

            text:text

        })

    });



    document.getElementById("title").value="";

    document.getElementById("text").value="";



    loadNotes();

}




// удалить заметку

async function deleteNote(id){


    if(!confirm("Удалить заметку?")){

        return;

    }


    await fetch(
        "/api/delete?id="+id
    );


    loadNotes();

}



// избранное

async function favoriteNote(id){


    await fetch(
        "/api/favorite?id="+id
    );


    loadNotes();

}



// редактирование

async function editNote(id){


    let note =
    allNotes.find(
        n=>n.id===id
    );


    let title =
    prompt(
        "Новое название:",
        note.title
    );


    if(title===null)
    return;



    let text =
    prompt(
        "Новый текст:",
        note.text
    );


    if(text===null)
    return;



    await fetch("/api/edit",{


        method:"POST",


        headers:{
            "Content-Type":"application/json"
        },


        body:JSON.stringify({

            id:id,

            title:title,

            text:text

        })

    });



    loadNotes();

}




// очистить всё

async function clearNotes(){


    if(!confirm("Удалить все заметки?"))

    return;



    await fetch(
        "/api/clear"
    );


    loadNotes();

}



// экспорт

function exportNotes(){


    window.open(
        "/api/export",
        "_blank"
    );


}



// запуск

window.onload = function(){

    loadNotes();

};
