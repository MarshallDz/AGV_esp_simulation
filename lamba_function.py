import json

def lambda_handler(event, context):
    # Ottieni lo stato attuale e l'evento dall'input
    current_state = event["queryStringParameters"]["current_state"]
    input_event = event["queryStringParameters"]["input_event"]

    # Gestione degli stati
    if current_state == 'in_stazione':
        if input_event == 'azione':
            new_state = 'navigazione'
            message = "Navigazione iniziata"
        else:
            new_state = 'in_stazione'
            message = "In stazione, in attesa di comando"

    elif current_state == 'navigazione':
        if input_event == 'azione':
            new_state = 'attesa_carico'
            message = "In attesa del carico"
        else:
            new_state = 'navigazione'
            message = "In navigazione, in attesa di comando"

    elif current_state == 'attesa_carico':
        if input_event == 'pot_up':
            new_state = 'caricato'
            message = "Carico completato"
        else:
            new_state = 'attesa_carico'
            message = "In carico, in attesa di comando"

    elif current_state == 'caricato':
        if input_event == 'azione':
            new_state = 'attesa_scarico'
            message = "In attesa dello scarico"
        else:
            new_state = 'caricato'
            message = "Caricato, in attesa di comando"

    elif current_state == 'attesa_scarico':
        if input_event == 'pot_down':
            new_state = 'in_stazione'
            message = "Scarico completato, in ritorno alla stazione"
        else:
            new_state = 'attesa_scarico'
            message = "In attesa scarico"

    elif current_state == 'emergenza':
        if input_event == 'azione':
            new_state = 'in_stazione'
            message = "Emergenza risolta, tornato in stazione"
        else:
            new_state = 'emergenza'
            message = "In emergenza, in attesa di risoluzione"

    # Gestione dell'evento di emergenza in qualsiasi stato
    if input_event == 'emergenza':
        new_state = 'emergenza'
        message = "Entrato in stato di emergenza"

    # Risposta che verrà restituita da AWS Lambda
    response = {
        'new_state': new_state,
        'message': message
    }

    return {
        'statusCode': 200,
        'body': json.dumps(response)
    }
