import json

def lambda_handler(event, context):
    # Ottieni lo stato attuale e l'evento dall'input
    current_state = event["queryStringParameters"]["current_state"]
    input_event = event["queryStringParameters"]["input_event"]

    # Gestione degli stati
    if current_state == 'in_stazione':
        if input_event == 'start_navigation':
            new_state = 'navigazione'
            message = "Navigazione iniziata"
        else:
            new_state = 'in_stazione'
            message = "In stazione, in attesa di comando"

    elif current_state == 'navigazione':
        if input_event == 'reach_load_station':
            new_state = 'carico'
            message = "Inizio carico"
        elif input_event == 'reach_unload_station':
            new_state = 'scarico'
            message = "Inizio scarico"
        else:
            new_state = 'navigazione'
            message = "Navigazione in corso"

    elif current_state == 'carico':
        if input_event == 'load_complete':
            new_state = 'navigazione'
            message = "Carico completato, navigazione verso stazione di scarico"
        else:
            new_state = 'carico'
            message = "In carico"

    elif current_state == 'scarico':
        if input_event == 'unload_complete':
            new_state = 'in_stazione'
            message = "Scarico completato, tornato in stazione"
        else:
            new_state = 'scarico'
            message = "In scarico"

    elif current_state == 'emergenza':
        if input_event == 'resolve_emergency':
            new_state = 'in_stazione'
            message = "Emergenza risolta, tornato in stazione"
        else:
            new_state = 'emergenza'
            message = "In emergenza, in attesa di risoluzione"

    # Gestione dell'evento di emergenza in qualsiasi stato
    if input_event == 'emergency':
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
